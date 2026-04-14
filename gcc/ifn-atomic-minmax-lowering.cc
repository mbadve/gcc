/* Lower internal function for atomic minmax to a CAS loop.
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

This pass lowers IFN_ATOMIC_FETCH_MINMAX calls to a Compare-and-Swap (CAS) loop.

A fetch_op call can be represented in the following manner:
	oldval = atomic_load (ptr);
	do {
	    newval = op (oldval, value);
	} while (!atomic_compare_exchange (ptr, &oldval, newval));
	return oldval;

This pass transforms the IFN to the following pattern:

	before_bb:
	  oldval_1 = atomic_load (ptr)
	  goto loop

	loop_bb:
	  oldval_2 = PHI<oldval_1 (before_bb), oldval_3 (loop_bb)>
	  newval = MIN/MAX (oldval_2, value)
	  success = CAS (ptr, &oldval, newval)
	  oldval_3 = *(&oldval)  // CAS updates oldval on failure
	  if (success) goto exit else goto loop

	exit_bb:
	  // return oldval_2 (or newval if op_fetch)

*/

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "target.h"
#include "rtl.h"
#include "tree.h"
#include "gimple.h"
#include "gimple-iterator.h"
#include "cfghooks.h"
#include "tree-pass.h"
#include "memmodel.h"
#include "tm_p.h"
#include "ssa.h"
#include "optabs.h"
#include "optabs-query.h"
#include "tree-ssanames.h"
#include "fold-const.h"
#include "cfganal.h"
#include "gimple-fold.h"
#include "tree-cfg.h"
#include "gimplify.h"
#include "gimple-ssa.h"
#include "tree-ssa-operands.h"
#include "builtins.h"
#include "internal-fn.h"
#include "tree-into-ssa.h"

/* Helper function to convert between types and insert into gimple sequence.
   Returns the SSA name holding the converted value.  */

static tree
insert_type_conversion (gimple_stmt_iterator *gsi, tree src_value,
			tree dest_type, enum tree_code convert_code = NOP_EXPR,
			bool insert_after = true)
{
  if (TREE_TYPE (src_value) == dest_type)
    return src_value;

  tree result = make_ssa_name (dest_type);
  gassign *convert_stmt = gimple_build_assign (result, convert_code, src_value);

  if (insert_after)
    gsi_insert_after (gsi, convert_stmt, GSI_NEW_STMT);
  else
    gsi_insert_before (gsi, convert_stmt, GSI_SAME_STMT);

  return result;
}

/* Check if optab is available for the atomic min/max operation.
   if optab for op_fetch not available, try for fetch_op.  */

static bool
can_expand_atomic_minmax (gcall *call_stmt)
{
  tree datatype = TREE_TYPE (TREE_TYPE (gimple_call_arg (call_stmt, 0)));
  bool is_min = tree_to_uhwi (gimple_call_arg (call_stmt, 3));
  bool is_fetch_op = tree_to_uhwi (gimple_call_arg (call_stmt, 4));

  machine_mode mode = TYPE_MODE (datatype);
  bool is_signed = !TYPE_UNSIGNED (datatype);

  optab op_optab
    = is_signed
	? (is_fetch_op
	     ? (is_min ? atomic_fetch_smin_optab : atomic_fetch_smax_optab)
	     : (is_min ? atomic_smin_fetch_optab : atomic_smax_fetch_optab))
	: (is_fetch_op
	     ? (is_min ? atomic_fetch_umin_optab : atomic_fetch_umax_optab)
	     : (is_min ? atomic_umin_fetch_optab : atomic_umax_fetch_optab));

  auto insn_code = optab_handler (op_optab, mode);
  if (insn_code != CODE_FOR_nothing) return true;

  /* Check if optab for reverse op is available.  */
  optab rev_op_optab
    = is_signed
	? (is_fetch_op
	     ? (is_min ? atomic_smin_fetch_optab : atomic_smax_fetch_optab)
	     : (is_min ? atomic_fetch_smin_optab : atomic_fetch_smax_optab))
	: (is_fetch_op
	     ? (is_min ? atomic_umin_fetch_optab : atomic_umax_fetch_optab)
	     : (is_min ? atomic_fetch_umin_optab : atomic_fetch_umax_optab));

  insn_code = optab_handler (rev_op_optab, mode);
  if (insn_code != CODE_FOR_nothing) return true;

  return false;
}

/* Lower a single IFN_ATOMIC_FETCH_MINMAX call to a CAS loop.  */

static void
lower_ifn_atomic_minmax_to_cas (gimple_stmt_iterator *iter, gcall *stmt)
{
  tree ptr = gimple_call_arg (stmt, 0);
  tree value = gimple_call_arg (stmt, 1);
  tree memorder = gimple_call_arg (stmt, 2);
  tree is_min = gimple_call_arg (stmt, 3);
  tree is_fetch_op = gimple_call_arg (stmt, 4);

  tree datatype = TREE_TYPE (TREE_TYPE (ptr));

  bool is_min_bool = tree_to_uhwi (is_min);
  tree_code cmp_code = is_min_bool ? MIN_EXPR : MAX_EXPR;

  /* Build the atomic builtins we need.  */
  int size_log2 = exact_log2 (tree_to_uhwi (TYPE_SIZE_UNIT (datatype)));
  built_in_function load_builtin_fn
    = (built_in_function) ((int) BUILT_IN_ATOMIC_LOAD_N + size_log2 + 1);
  tree load_fn = builtin_decl_explicit (load_builtin_fn);

  built_in_function cas_builtin_fn
    = (built_in_function) ((int) BUILT_IN_ATOMIC_COMPARE_EXCHANGE_N + size_log2
			   + 1);
  tree cas_fn = builtin_decl_explicit (cas_builtin_fn);

  /* Atomic operations expect unsigned values as arguments.  Calcualte the
     corresponding unsigned type.  */
  tree unsigned_type = unsigned_type_for (datatype);

  /* Insert atomic load before the IFN.  We will be using the IFN as a split
     point later to insert the loop body.  Since the load is not part of the
     loop body, we insert it before the IFN.  */
  tree initial_oldval = make_ssa_name (unsigned_type);
  gcall *load_call
    = gimple_build_call (load_fn, 2, ptr,
			 build_int_cst (integer_type_node, MEMMODEL_RELAXED));
  gimple_call_set_lhs (load_call, initial_oldval);
  gsi_insert_before (iter, load_call, GSI_SAME_STMT);

  /* Save LHS before removing the IFN.  Need it later to assign the result.  */
  tree lhs = gimple_call_lhs (stmt);

  /* Build the loop body.  We will be inserting this after the split point.  */
  gimple_seq loop_body = NULL;
  gimple_stmt_iterator gsi_loop = gsi_last (loop_body);

  /* Create a SSA name for the loop variable.  This will be the PHI node in the
     loop body.  */
  tree loop_oldval = make_ssa_name (unsigned_type);

  /* Convert to original type for MIN/MAX if needed.  */
  tree oldval_typed
    = TYPE_UNSIGNED (datatype)
	? loop_oldval
	: insert_type_conversion (&gsi_loop, loop_oldval, datatype);

  /* Convert value to the same type as oldval_typed.  */
  tree value_typed = value;
  if (TREE_TYPE (value) != datatype)
    value_typed = insert_type_conversion (&gsi_loop, value, datatype);

  /* Compute newval = MIN/MAX (oldval, value).  */
  tree newval = make_ssa_name (datatype);
  gassign *minmax_assign
    = gimple_build_assign (newval, cmp_code, oldval_typed, value_typed);
  gsi_insert_after (&gsi_loop, minmax_assign, GSI_NEW_STMT);

  /* Convert newval to unsigned for CAS if needed.  */
  tree newval_unsigned
    = TYPE_UNSIGNED (datatype)
	? newval
	: insert_type_conversion (&gsi_loop, newval, unsigned_type);

  /* For CAS, we need a temporary addressable variable.  CAS expects oldval to
     be addressable.  Therefore, using the SSA name won't work.  */
  tree cas_expected = create_tmp_var (unsigned_type, "cas_expected");
  TREE_ADDRESSABLE (cas_expected) = 1;

  /* Store the current oldval into the addressable variable.  */
  gsi_insert_after (&gsi_loop, gimple_build_assign (cas_expected, loop_oldval),
		    GSI_NEW_STMT);

  tree cas_result = make_ssa_name (boolean_type_node);
  tree cas_expected_addr = build_fold_addr_expr (cas_expected);

  gcall *cas_call
    = gimple_build_call (cas_fn, 6, ptr,     /* address to update.  */
			 cas_expected_addr,  /* expected value.  */
			 newval_unsigned,    /* desired value.  */
			 boolean_false_node, /* weak.  */
			 memorder,	     /* success order.  */
			 build_int_cst (integer_type_node,
				MEMMODEL_RELAXED));  /* failure order.  */
  gimple_call_set_lhs (cas_call, cas_result);
  gsi_insert_after (&gsi_loop, cas_call, GSI_NEW_STMT);

  /* On failure (desired != expected), CAS updated the expected value.  Read it
     back into SSA form.  */
  tree cas_oldval_updated = make_ssa_name (unsigned_type);
  gsi_insert_after (&gsi_loop,
		    gimple_build_assign (cas_oldval_updated, cas_expected),
		    GSI_NEW_STMT);

  /* Add conditional to end of loop body.  Currently, the conditional statement
     has no successors.  We will be adding loop_bb and exit_bb as successors
     later.  */
  gcond *loop_cond = gimple_build_cond (NE_EXPR, cas_result,
					build_int_cst (boolean_type_node, 0),
					NULL_TREE, NULL_TREE);
  gsi_insert_after (&gsi_loop, loop_cond, GSI_NEW_STMT);

  /* Get the previous statement in the basic block.  We will be using it to
     split the block after the atomic load.

     Currently, the iter points to the IFN.  The bb looks something like:

     <previous_code>
     ->
     atomic_load (ptr)
     ->
     IFN_ATOMIC_FETCH_MINMAX <- gsi
     ->
     <following_code>

     We need to remove the IFN and replace it with the loop body.  To do this,
     we split at the IFN, and further split the resultant edge to insert the
     loop body, resulting in:

     before_bb (contains <previous_code> and atomic_load (ptr))
     ->
     loop_bb (contains the MIN/MAX operation and the CAS operation)
     ->
     after_bb (contains <following_code>)

   */

  gimple_stmt_iterator prev_gsi = *iter;
  gsi_prev (&prev_gsi);

  /* Remove the IFN.  */
  gsi_remove (iter, true);

  edge e = split_block (gsi_bb (prev_gsi), gsi_stmt (prev_gsi));
  basic_block exit_bb = e->dest;
  basic_block loop_bb = split_edge (e);

  /* Create PHI node for oldval.  One edge will come from the predecessor
     (initial_oldval), and the other will come from the loop back
     (cas_oldval_updated).  */
  gphi *loop_phi = create_phi_node (loop_oldval, loop_bb);
  add_phi_arg (loop_phi, initial_oldval, single_pred_edge (loop_bb),
	       UNKNOWN_LOCATION);

  /* Insert the loop body into loop_bb.  */
  set_bb_seq (loop_bb, loop_body);

  /* Set the basic block for all statements in the loop body to ensure they
     are correctly assigned to loop_bb.  */
  for (gimple_stmt_iterator si = gsi_start_bb (loop_bb); !gsi_end_p (si);
       gsi_next (&si))
    gimple_set_bb (gsi_stmt (si), loop_bb);

  /* loop_bb currently has a fallthrough edge to exit_bb, due to split_edge.
     We need to remove it and create two conditional edges.  */
  remove_edge (single_succ_edge (loop_bb));
  edge true_edge = make_edge (loop_bb, exit_bb, EDGE_TRUE_VALUE);
  edge false_edge = make_edge (loop_bb, loop_bb, EDGE_FALSE_VALUE);

  false_edge->probability = profile_probability::unlikely ();
  true_edge->probability = false_edge->probability.invert ();

  /* Add the loop-back phi argument.  */
  add_phi_arg (loop_phi, cas_oldval_updated, false_edge, UNKNOWN_LOCATION);

  bool is_fetch_op_bool = tree_to_uhwi (is_fetch_op);
  if (lhs)
    {
      gimple_stmt_iterator exit_gsi = gsi_after_labels (exit_bb);
      tree result;

      if (is_fetch_op_bool)
	{
	  /* fetch_min/fetch_max: return the old value.  */
	  result = TYPE_UNSIGNED (datatype)
		     ? loop_oldval
		     : insert_type_conversion (&exit_gsi, loop_oldval, datatype,
					       NOP_EXPR, false);
	}
      else
	{
	  /* min_fetch/max_fetch: return the new value.  */
	  result = newval;
	}

      gassign *return_result = gimple_build_assign (lhs, result);
      gsi_insert_before (&exit_gsi, return_result, GSI_SAME_STMT);
    }
}

static unsigned int
lower_ifn_atomic_minmax (void)
{
  basic_block bb;
  bool changed = false;

  /* Iterate through all basic blocks looking for IFN calls to lower.  */
  FOR_EACH_BB_FN (bb, cfun)
    {
      for (gimple_stmt_iterator gsi = gsi_start_bb (bb); !gsi_end_p (gsi);)
	{
	  gimple *stmt = gsi_stmt (gsi);

	  if (is_gimple_call (stmt))
	    {
	      gcall *call_stmt = as_a<gcall *> (stmt);
	      if (gimple_call_internal_p (call_stmt, IFN_ATOMIC_FETCH_MINMAX))
		{
		  /* Only lower to CAS if the target doesn't support the optab
		   */
		  if (!can_expand_atomic_minmax (call_stmt))
		    {
		      lower_ifn_atomic_minmax_to_cas (&gsi, call_stmt);
		      changed = true;
		      break;
		    }
		  /* Otherwise, leave it for expand_ATOMIC_FETCH_MINMAX.  */
		}
	    }
	  gsi_next (&gsi);
	}
    }

  return changed ? TODO_update_ssa | TODO_cleanup_cfg : 0;
}

static bool
gate_lower_ifn_atomic_minmax (void)
{
  return true;
}

namespace {

const pass_data pass_data_lower_ifn_atomic_minmax = {
  GIMPLE_PASS,		     /* type.  */
  "lower-ifn-atomic-minmax", /* name.  */
  OPTGROUP_NONE,	     /* optinfo_flags.  */
  TV_NONE,		     /* tv_id.  */
  (PROP_cfg | PROP_ssa),     /* properties_required.  */
  0,			     /* properties_provided.  */
  0,			     /* properties_destroyed.  */
  0,			     /* todo_flags_start.  */
  TODO_update_ssa	     /* todo_flags_finish.  */
};

class pass_lower_ifn_atomic_minmax : public gimple_opt_pass
{
public:
  pass_lower_ifn_atomic_minmax (gcc::context *ctxt)
    : gimple_opt_pass (pass_data_lower_ifn_atomic_minmax, ctxt)
  {}

  /* opt_pass methods: */
  bool gate (function *) final override
  {
    return gate_lower_ifn_atomic_minmax ();
  }
  unsigned int execute (function *) final override
  {
    return lower_ifn_atomic_minmax ();
  }
}; // class pass_lower_ifn_atomic_minmax

} // namespace

gimple_opt_pass *
make_pass_lower_ifn_atomic_minmax (gcc::context *ctxt)
{
  return new pass_lower_ifn_atomic_minmax (ctxt);
}
