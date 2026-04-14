/* Transform CAS loops implementing fetch_min/max to IFN_ATOMIC_FETCH_MINMAX.
   Copyright The GNU Toolchain Authors.

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GCC is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING3.  If not see
   <http://www.gnu.org/licenses/>.

This pass detects CAS loops implementing atomic MIN/MAX operations and
transforms them to IFN_ATOMIC_FETCH_MINMAX calls.

Pattern detected:
	expected = atomic_load (ptr)
	loop:
	    desired = MIN/MAX (expected, value)
	    success = compare_exchange (ptr, &expected, desired, mem_order)
	    if (success) exit loop
	    goto loop
	return expected

Transformed to:
    result = IFN_ATOMIC_FETCH_MINMAX (ptr,
				      value, // value to compare against
				      mem_order, // success memory order
				      is_min,
				      1, // fetch_op
				      is_signed)
    return result

*/

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "tree-core.h"
#include "tree.h"
#include "gimple.h"
#include "tree-pass.h"
#include "ssa.h"
#include "gimple-iterator.h"
#include "tree-cfg.h"
#include "tree-ssa.h"
#include "cfgloop.h"
#include "fold-const.h"
#include "cfganal.h"
#include "internal-fn.h"

/* Helper function for following SSA definitions through copies and casts to
   find the base value.
   For example: if _1 = x, and _2 = (int) _1, get_ssa_base_value (_2)
   returns x.  Returns the original value if it's not an SSA name or
   doesn't have a simple copy/cast chain.  */
static tree
get_ssa_base_value (tree name)
{
  if (TREE_CODE (name) != SSA_NAME)
    return name;

  gimple *def_stmt = SSA_NAME_DEF_STMT (name);

  /* Follow casts.  */
  if (gimple_assign_cast_p (def_stmt))
    return get_ssa_base_value (gimple_assign_rhs1 (def_stmt));

  /* Follow simple copies.  */
  if (gimple_assign_single_p (def_stmt))
    {
      tree rhs = gimple_assign_rhs1 (def_stmt);
      if (TREE_CODE (rhs) == SSA_NAME || TREE_CODE (rhs) == VAR_DECL
	  || TREE_CODE (rhs) == PARM_DECL)
	return get_ssa_base_value (rhs);
    }

  return name;
}

/* Helper function for comparing two values for equality, following SSA copies
   and casts.  */
static bool
values_equal_p (tree a, tree b)
{
  if (operand_equal_p (a, b, 0))
    return true;

  tree base_a = get_ssa_base_value (a);
  tree base_b = get_ssa_base_value (b);

  return operand_equal_p (base_a, base_b, 0);
}

/* Returns true if CALL is an atomic compare and swap call, either in the
   form of an internal function or builtin.  */
static bool
is_atomic_cas_call_p (gcall *call)
{
  if (gimple_call_internal_p (call, IFN_ATOMIC_COMPARE_EXCHANGE))
    return true;

  tree fndecl = gimple_call_fndecl (call);
  if (fndecl && fndecl_built_in_p (fndecl, BUILT_IN_NORMAL))
    {
      enum built_in_function fncode = DECL_FUNCTION_CODE (fndecl);
      return (fncode >= BUILT_IN_ATOMIC_COMPARE_EXCHANGE_N
	      && fncode <= BUILT_IN_ATOMIC_COMPARE_EXCHANGE_16);
    }
  return false;
}

/* Find the CAS call in LOOP, if any.  */
static gcall *
find_cas_in_loop (class loop *loop)
{
  basic_block *bbs = get_loop_body (loop);
  gcall *cas_call = NULL;

  for (unsigned i = 0; i < loop->num_nodes && !cas_call; i++)
    {
      for (gimple_stmt_iterator gsi = gsi_start_bb (bbs[i]); !gsi_end_p (gsi);
	   gsi_next (&gsi))
	{
	  gimple *stmt = gsi_stmt (gsi);
	  if (gcall *call = dyn_cast<gcall *> (stmt))
	    {
	      if (is_atomic_cas_call_p (call))
		{
		  cas_call = call;
		  break;
		}
	    }
	}
    }

  free (bbs);
  return cas_call;
}

/* Find the conditional statement that uses the CAS success flag to control
   the loop.  Returns NULL if not found or invalid.  */
static gcond *
find_cas_exit_cond (gcall *cas_call, tree cas_result, class loop *loop)
{
  tree bool_result = NULL_TREE;

  /* Handle IFN.  */
  if (gimple_call_internal_p (cas_call, IFN_ATOMIC_COMPARE_EXCHANGE))
    {
      /* IFN returns complex type: (expected, success_flag).  The success
	 flag is extracted via IMAGPART_EXPR and then cast to bool.  */
      for (gimple *use_stmt : gather_imm_use_stmts (cas_result))
	{
	  /* Skip non-assignment statements.  */
	  if (!is_gimple_assign (use_stmt)
	      || gimple_assign_rhs_code (use_stmt) != IMAGPART_EXPR)
	    continue;

	  tree success_flag = gimple_assign_lhs (use_stmt);
	  for (gimple *cast_use : gather_imm_use_stmts (success_flag))
	    {
	      if (gimple_assign_cast_p (cast_use))
		{
		  bool_result = gimple_assign_lhs (cast_use);
		  break;
		}
	    }
	  break;
	}
    }
  /* Handle builtin.  */
  else
    bool_result = cas_result;

  if (!bool_result)
    return NULL;

  /* Find the conditional that uses the bool result.  */
  gcond *cond = NULL;
  for (gimple *use_stmt : gather_imm_use_stmts (bool_result))
    {
      if ((cond = dyn_cast<gcond *> (use_stmt)))
	break;
      /* Bool might be negated before use.  */
      if (is_gimple_assign (use_stmt)
	  && gimple_assign_rhs_code (use_stmt) == BIT_NOT_EXPR)
	{
	  tree negated = gimple_assign_lhs (use_stmt);
	  for (gimple *neg_use : gather_imm_use_stmts (negated))
	    {
	      if ((cond = dyn_cast<gcond *> (neg_use)))
		break;
	    }
	  if (cond)
	    break;
	}
    }

  if (!cond)
    return NULL;

  /* Verify the conditional is in the loop and controls single exit.  */
  basic_block cond_bb = gimple_bb (cond);
  if (!flow_bb_inside_loop_p (loop, cond_bb))
    return NULL;

  edge exit_edge = single_exit (loop);
  if (!exit_edge || exit_edge->src != cond_bb)
    return NULL;

  return cond;
}

/* Check if VALUE defined inside LOOP is used outside LOOP.  */
static bool
value_escapes_loop_p (tree value, class loop *loop)
{
  if (TREE_CODE (value) != SSA_NAME)
    return false;

  /* Find instances of defs inside the loop.  */
  gimple *def_stmt = SSA_NAME_DEF_STMT (value);
  basic_block def_bb = gimple_bb (def_stmt);

  if (!def_bb || !flow_bb_inside_loop_p (loop, def_bb))
    return false;

  /* Find uses of those defs outside the loop.  */
  for (gimple *use_stmt : gather_imm_use_stmts (value))
    {
      basic_block use_bb = gimple_bb (use_stmt);
      if (use_bb && !flow_bb_inside_loop_p (loop, use_bb))
	return true;
    }
  return false;
}

/* For IFN CAS, the result is a complex type where REALPART holds the
   'expected' value (the last value read from memory before a successful CAS).
   This function finds the SSA name representing 'expected' and returns it
   if the value escapes the loop.  Returns NULL_TREE otherwise.  */
static tree
find_expected_out_ssa (tree cas_result, class loop *loop)
{
  for (gimple *use_stmt : gather_imm_use_stmts (cas_result))
    {
      if (!is_gimple_assign (use_stmt)
	  || gimple_assign_rhs_code (use_stmt) != REALPART_EXPR)
	continue;

      tree realpart = gimple_assign_lhs (use_stmt);

      if (value_escapes_loop_p (realpart, loop))
	return realpart;

      /* Check for cast after REALPART_EXPR.  */
      for (gimple *cast_use : gather_imm_use_stmts (realpart))
	{
	  if (gimple_assign_cast_p (cast_use))
	    {
	      tree cast_result = gimple_assign_lhs (cast_use);
	      if (value_escapes_loop_p (cast_result, loop))
		return cast_result;
	    }
	}
    }
  return NULL_TREE;
}

/* Check if statements (other than the CAS call) in the loop have side
   effects or if values defined within the loop escape its scope.

   EXPECTED_OUT is set to the SSA name of the 'expected' value returned
   by the REALPART of a CAS IFN return.  */
static bool
loop_has_side_effects_p (class loop *loop, gcall *cas_call, tree cas_result,
		       tree *expected_out)
{
  basic_block *bbs = get_loop_body (loop);
  bool has_side_effects = false;
  tree cas_expected = NULL_TREE;

  /* For IFN, check if 'expected' (extracted via REALPART) escapes.  */
  if (gimple_call_internal_p (cas_call, IFN_ATOMIC_COMPARE_EXCHANGE))
    cas_expected = find_expected_out_ssa (cas_result, loop);

  for (unsigned i = 0; i < loop->num_nodes && !has_side_effects; i++)
    {
      for (gimple_stmt_iterator gsi = gsi_start_bb (bbs[i]); !gsi_end_p (gsi);
	   gsi_next (&gsi))
	{
	  gimple *stmt = gsi_stmt (gsi);

	  /* Skip the CAS call itself.  */
	  if (stmt == cas_call)
	    continue;

	  if (gimple_has_side_effects (stmt))
	    {
	      has_side_effects = true;
	      break;
	    }

	  tree lhs = gimple_get_lhs (stmt);
	  if (!lhs)
	    continue;

	  /* Allow 'expected' to escape, handled later.  */
	  if (cas_expected && lhs == cas_expected)
	    continue;

	  if (value_escapes_loop_p (lhs, loop))
	    {
	      has_side_effects = true;
	      break;
	    }
	}

      for (gphi_iterator psi = gsi_start_phis (bbs[i]); !gsi_end_p (psi);
	   gsi_next (&psi))
	{
	  gphi *phi = psi.phi ();
	  tree result = gimple_phi_result (phi);

	  if (value_escapes_loop_p (result, loop))
	    {
	      has_side_effects = true;
	      break;
	    }
	}
    }

  free (bbs);

  if (!has_side_effects)
    *expected_out = cas_expected;

  return has_side_effects;
}

/* Extract CAS call arguments.  For builtins, dereference the expected
   pointer.  Returns true on success.  */
static bool
extract_cas_arguments (gcall *cas_call, tree *ptr_out, tree *expected_out,
		       tree *desired_out, tree *success_model_out)
{
  *ptr_out = gimple_call_arg (cas_call, 0);
  *desired_out = gimple_call_arg (cas_call, 2);
  *success_model_out = gimple_call_arg (cas_call, 4);

  if (gimple_call_internal_p (cas_call, IFN_ATOMIC_COMPARE_EXCHANGE))
    {
      *expected_out = gimple_call_arg (cas_call, 1);
      return true;
    }
  else
    {
      tree expected_ref = gimple_call_arg (cas_call, 1);
      if (TREE_CODE (expected_ref) != ADDR_EXPR)
	return false;
      *expected_out = TREE_OPERAND (expected_ref, 0);
      return true;
    }
}

/* MIN/MAX pattern information.  */
struct minmax_pattern
{
  enum pattern_type
  {
    NONE, /* No MIN/MAX pattern found.  */
    PHI, /* MIN/MAX pattern found in a PHI node from a conditional branch.  */
    EXPR /* MIN/MAX pattern found in a MIN_EXPR/MAX_EXPR node.  */
  } type;
  union
  {
    gphi *phi; /* PHI node from a conditional branch.  */
    gassign *assign; /* MIN_EXPR/MAX_EXPR node.  */
  } stmt;
};

/* Trace from DESIRED value to find MIN/MAX pattern (PHI or MIN_EXPR/MAX_EXPR).
   Returns pattern info or NONE if not found.  */
static minmax_pattern
find_minmax_pattern (tree desired)
{
  minmax_pattern result = {minmax_pattern::NONE, {nullptr}};

  if (TREE_CODE (desired) != SSA_NAME)
    return result;

  gimple *def_stmt = SSA_NAME_DEF_STMT (desired);

  /* Skip casts.  */
  tree stripped = get_ssa_base_value (desired);
  if (TREE_CODE (stripped) == SSA_NAME && stripped != desired)
    def_stmt = SSA_NAME_DEF_STMT (stripped);

  if (is_gimple_assign (def_stmt))
    {
      tree_code code = gimple_assign_rhs_code (def_stmt);

      if (code == MIN_EXPR || code == MAX_EXPR)
	{
	  result.type = minmax_pattern::EXPR;
	  result.stmt.assign = as_a<gassign *> (def_stmt);
	  return result;
	}

      /* Follow MEM_REF to find address PHI.  */
      if (gimple_assign_single_p (def_stmt) && code == MEM_REF)
	{
	  tree rhs = gimple_assign_rhs1 (def_stmt);
	  tree addr = TREE_OPERAND (rhs, 0);
	  if (TREE_CODE (addr) == SSA_NAME)
	    def_stmt = SSA_NAME_DEF_STMT (addr);
	  else
	    return result;
	}
    }

  if (gimple_code (def_stmt) == GIMPLE_PHI)
    {
      result.type = minmax_pattern::PHI;
      result.stmt.phi = as_a<gphi *> (def_stmt);
    }

  return result;
}

/* Analyze MIN_EXPR/MAX_EXPR to extract the comparison value (VALUE_OUT) and
   min/max type.  EXPECTED is the value passed to CAS, which is compared against
   VALUE_OUT.  */
static bool
analyze_minmax_expr (gassign *assign, tree expected, tree *value_out,
		     bool *is_min_out)
{
  tree_code code = gimple_assign_rhs_code (assign);
  if (code != MIN_EXPR && code != MAX_EXPR)
    return false;

  tree op1 = gimple_assign_rhs1 (assign);
  tree op2 = gimple_assign_rhs2 (assign);

  if (values_equal_p (op1, expected))
    *value_out = op2;
  else if (values_equal_p (op2, expected))
    *value_out = op1;
  else
    return false;

  *is_min_out = (code == MIN_EXPR);
  return true;
}

/* Analyze PHI node implementing MIN/MAX selection.  EXPECTED is the value
   passed to CAS, which is compared against VALUE_OUT.  Use these with the PHI
   args to determine the operation (MIN or MAX) being performed.  */
static bool
analyze_minmax_phi (gphi *phi, tree expected, tree *value_out, bool *is_min_out)
{
  if (gimple_phi_num_args (phi) != 2)
    return false;

  tree arg0 = gimple_phi_arg_def (phi, 0);
  tree arg1 = gimple_phi_arg_def (phi, 1);
  edge edge0 = gimple_phi_arg_edge (phi, 0);
  edge edge1 = gimple_phi_arg_edge (phi, 1);

  /* Dereference address arguments.  */
  if (TREE_CODE (arg0) == ADDR_EXPR)
    arg0 = TREE_OPERAND (arg0, 0);
  if (TREE_CODE (arg1) == ADDR_EXPR)
    arg1 = TREE_OPERAND (arg1, 0);

  basic_block phi_bb = gimple_bb (phi);
  basic_block dom = get_immediate_dominator (CDI_DOMINATORS, phi_bb);
  if (!dom)
    return false;

  gimple_stmt_iterator gsi = gsi_last_nondebug_bb (dom);
  if (gsi_end_p (gsi))
    return false;

  gcond *cond = dyn_cast<gcond *> (gsi_stmt (gsi));
  if (!cond)
    return false;

  tree cond_lhs = gimple_cond_lhs (cond);
  tree cond_rhs = gimple_cond_rhs (cond);
  enum tree_code cond_code = gimple_cond_code (cond);

  edge true_edge, false_edge;
  extract_true_false_edges_from_block (dom, &true_edge, &false_edge);

  /* Skip intermediate empty blocks.  */
  if (true_edge->dest != phi_bb && single_succ_p (true_edge->dest))
    true_edge = single_succ_edge (true_edge->dest);
  if (false_edge->dest != phi_bb && single_succ_p (false_edge->dest))
    false_edge = single_succ_edge (false_edge->dest);

  /* Map PHI arguments to TRUE/FALSE paths.  */
  tree true_arg, false_arg;
  if (true_edge == edge0)
    {
      true_arg = arg0;
      false_arg = arg1;
    }
  else if (true_edge == edge1)
    {
      true_arg = arg1;
      false_arg = arg0;
    }
  else
    return false;

  /* Determine MIN vs MAX based on condition and which value is selected.
     For "LHS op RHS":
       - TRUE selects LHS: LT/LE -> MIN, GT/GE -> MAX.
       - TRUE selects RHS: LT/LE -> MAX, GT/GE -> MIN.  */
  bool is_min;
  if (values_equal_p (true_arg, cond_lhs)
      && values_equal_p (false_arg, cond_rhs))
    is_min = (cond_code == LT_EXPR || cond_code == LE_EXPR);
  else if (values_equal_p (true_arg, cond_rhs)
	   && values_equal_p (false_arg, cond_lhs))
    is_min = (cond_code == GT_EXPR || cond_code == GE_EXPR);
  else
    return false;

  /* Identify which operand is 'expected' and which is 'value'.  */
  tree value;
  if (values_equal_p (cond_lhs, expected))
    value = cond_rhs;
  else if (values_equal_p (cond_rhs, expected))
    value = cond_lhs;
  else
    return false;

  *value_out = value;
  *is_min_out = is_min;
  return true;
}

/* Replace uses of CAS result to make the loop exit.
   For IFN, replace IMAGPART with 1 (success) and REALPART with EXPECTED_VALUE.
   For builtins, replace return with 1.  */
static void
replace_cas_uses_for_exit (gcall *cas_call, tree cas_result,
						tree expected_value)
{
  if (gimple_call_internal_p (cas_call, IFN_ATOMIC_COMPARE_EXCHANGE))
    {
      for (gimple *use_stmt : gather_imm_use_stmts (cas_result))
	{
	  if (!is_gimple_assign (use_stmt))
	    continue;

	  tree_code code = gimple_assign_rhs_code (use_stmt);
	  if (code != IMAGPART_EXPR && code != REALPART_EXPR)
	    continue;

	  tree lhs = gimple_assign_lhs (use_stmt);
	  tree val;

	  if (code == IMAGPART_EXPR)
	    val = build_one_cst (TREE_TYPE (lhs));
	  else if (expected_value)
	    val = expected_value;
	  else
	    val = build_zero_cst (TREE_TYPE (lhs));

	  gimple *new_stmt = gimple_build_assign (lhs, val);
	  gimple_stmt_iterator use_gsi = gsi_for_stmt (use_stmt);
	  gsi_replace (&use_gsi, new_stmt, true);
	}
    }
  else
    {
      /* Builtin: replace bool result with 1 (success).  */
      for (gimple *use_stmt : gather_imm_use_stmts (cas_result))
	{
	  gimple_stmt_iterator use_gsi = gsi_for_stmt (use_stmt);

	  if (is_gimple_assign (use_stmt))
	    {
	      tree lhs = gimple_assign_lhs (use_stmt);
	      gimple *new_stmt
		= gimple_build_assign (lhs, build_one_cst (TREE_TYPE (lhs)));
	      gsi_replace (&use_gsi, new_stmt, true);
	    }
	  /* For conditionals using the success flag, replace with a dummy
	     if (true == true) condition to force always exit.  */
	  else if (gcond *cond_stmt = dyn_cast<gcond *> (use_stmt))
	    {
	      gimple_cond_set_lhs (cond_stmt, boolean_true_node);
	      gimple_cond_set_rhs (cond_stmt, boolean_true_node);
	      gimple_cond_set_code (cond_stmt, EQ_EXPR);
	      update_stmt (cond_stmt);
	    }
	}
    }
}

/* Detect CAS loop implementing MIN/MAX and transform to
   IFN_ATOMIC_FETCH_MINMAX.  Returns true if transformed.  */
static bool
transform_cas_loop_to_minmax (class loop *loop)
{
  /* Check if the loop contains a CAS call.  It can be either an IFN or a
     builtin.  */
  gcall *cas_call = find_cas_in_loop (loop);
  if (!cas_call)
    return false;

  tree cas_result = gimple_call_lhs (cas_call);

  /* If the CAS result isn't captured, it cannot be a CAS loop.  */
  if (!cas_result)
    return false;

  /* Verify CAS controls loop exit.  */
  gcond *loop_cond = find_cas_exit_cond	(cas_call, cas_result, loop);
  if (!loop_cond)
    return false;

  /* Check side effects and ensure SSA names defined within the loop do not
     escape its scope.

     CAS IFNs return a complex type where REALPART holds the 'expected' value
     ie. the last value read from memory before a successful CAS.  This value
     might escape the loop as CAS loops implementing fetch_min/max will return
     this after a successful CAS.

     This is acceptable since it matches the return value of
     IFN_ATOMIC_FETCH_MINMAX when fetch_op=1.  This will be overwritten by the
     result of IFN_ATOMIC_FETCH_MINMAX later.  */

  tree expected_out = NULL_TREE;
  if (loop_has_side_effects_p (loop, cas_call, cas_result, &expected_out))
    return false;

  tree ptr, expected, desired, success_model;
  if (!extract_cas_arguments (cas_call, &ptr, &expected, &desired,
			      &success_model))
    return false;

  /* Find MIN/MAX pattern.  */
  minmax_pattern pattern = find_minmax_pattern (desired);
  if (pattern.type == minmax_pattern::NONE)
    return false;

  tree value;
  bool is_min;

  if (pattern.type == minmax_pattern::PHI)
    {
      if (!analyze_minmax_phi (pattern.stmt.phi, expected, &value, &is_min))
	return false;
    }
  else if (pattern.type == minmax_pattern::EXPR)
    {
      if (!analyze_minmax_expr (pattern.stmt.assign, expected, &value, &is_min))
	return false;
    }
  else
    return false;

  /* Build IFN_ATOMIC_FETCH_MINMAX call.  */
  tree data_type = TREE_TYPE (TREE_TYPE (ptr));
  bool is_signed = !TYPE_UNSIGNED (data_type);

  tree is_min_tree = build_int_cst (integer_type_node, is_min ? 1 : 0);
  tree is_fetch_op = build_int_cst (integer_type_node, 1);
  tree is_signed_tree = build_int_cst (integer_type_node, is_signed ? 1 : 0);

  gcall *ifn_call
    = gimple_build_call_internal (IFN_ATOMIC_FETCH_MINMAX, 6, ptr, value,
				  success_model, is_min_tree, is_fetch_op,
				  is_signed_tree);

  gimple_stmt_iterator cas_gsi = gsi_for_stmt (cas_call);
  bool is_ifn = gimple_call_internal_p (cas_call, IFN_ATOMIC_COMPARE_EXCHANGE);

  /* For IFN CAS: capture return if 'expected' (REALPART) escapes.
     For builtin CAS: always capture return and store into expected variable,
     since post-loop reads of expected should get the final value.  */
  tree minmax_result = NULL_TREE;

  if (is_ifn)
    {
      /* IFN case: only capture if 'expected' (REALPART) escapes.  */
      if (expected_out)
	{
	  tree result_type = TREE_TYPE (TREE_TYPE (cas_result));
	  minmax_result = make_ssa_name (result_type);
	  gimple_call_set_lhs (ifn_call, minmax_result);
	}
    }
  else
    {
      /* Builtin case: always capture return and store into 'expected'.
	 The expected variable may be read after the loop.  */
      minmax_result = make_ssa_name (data_type);
      gimple_call_set_lhs (ifn_call, minmax_result);
    }

  /* Insert FETCH_MINMAX before CAS.  */
  gsi_insert_before (&cas_gsi, ifn_call, GSI_SAME_STMT);


  if (!is_ifn && minmax_result)
    {
      gimple *store_stmt = gimple_build_assign (expected, minmax_result);
      gsi_insert_before (&cas_gsi, store_stmt, GSI_SAME_STMT);
    }

  /* Fixup CAS uses to make loop exit.  */
  replace_cas_uses_for_exit (cas_call, cas_result, minmax_result);

  /* Remove CAS.  */
  gsi_remove (&cas_gsi, true);

  if (dump_file)
    fprintf (dump_file, "Transformed CAS loop to atomic %s\n",
	     is_min ? "MIN" : "MAX");

  return true;
}

static unsigned int
transform_cas_loops_to_minmax (void)
{
  bool changed = false;
  calculate_dominance_info (CDI_DOMINATORS);

  if (!loops_state_satisfies_p (LOOPS_HAVE_RECORDED_EXITS))
    record_loop_exits ();

  for (auto loop : loops_list (cfun, LI_FROM_INNERMOST))
    {
      if (loop->header)
	changed |= transform_cas_loop_to_minmax (loop);
    }

  if (changed)
    return TODO_cleanup_cfg | TODO_update_ssa;

  return 0;
}

static bool
gate_transform_cas_loops (void)
{
  return optimize >= 2;
}

namespace {

const pass_data pass_data_transform_cas_loops_to_minmax = {
  GIMPLE_PASS,		 /* type.  */
  "cas-to-minmax",	 /* name.  */
  OPTGROUP_LOOP,	 /* optinfo_flags.  */
  TV_NONE,		 /* tv_id.  */
  (PROP_cfg | PROP_ssa), /* properties_required.  */
  0,			 /* properties_provided.  */
  0,			 /* properties_destroyed.  */
  0,			 /* todo_flags_start.  */
  0			 /* todo_flags_finish.  */
};

class pass_transform_cas_loops_to_minmax : public gimple_opt_pass
{
public:
  pass_transform_cas_loops_to_minmax (gcc::context *ctxt)
    : gimple_opt_pass (pass_data_transform_cas_loops_to_minmax, ctxt)
  {}

  bool gate (function *) final override { return gate_transform_cas_loops (); }

  unsigned int execute (function *) final override
  {
    return transform_cas_loops_to_minmax ();
  }
}; // class pass_transform_cas_loops_to_minmax

} // anonymous namespace

gimple_opt_pass *
make_pass_transform_cas_loops_to_minmax (gcc::context *ctxt)
{
  return new pass_transform_cas_loops_to_minmax (ctxt);
}
