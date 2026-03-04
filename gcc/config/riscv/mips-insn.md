;; Machine description for MIPS custom instructions.
;; Copyright (C) 2025 Free Software Foundation, Inc.

;; This file is part of GCC.

;; GCC is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 3, or (at your option)
;; any later version.

;; GCC is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with GCC; see the file COPYING3.  If not see
;; <http://www.gnu.org/licenses/>.

(define_insn "*mov<GPR:mode><X:mode>cc_bitmanip"
  [(set (match_operand:GPR 0 "register_operand" "=r")
	(if_then_else:GPR (any_eq:X (match_operand:X 1 "register_operand" "r")
				    (match_operand:X 2 "const_0_operand" "J"))
			  (match_operand:GPR 3 "reg_or_0_operand" "rJ")
			  (match_operand:GPR 4 "reg_or_0_operand" "rJ")))]
  "TARGET_XMIPSCMOV"
{
  enum rtx_code code = <CODE>;
  if (code == NE)
    return "mips.ccmov\t%0,%1,%z3,%z4";
  else
    return "mips.ccmov\t%0,%1,%z4,%z3";
}
[(set_attr "type" "condmove")
 (set_attr "mode" "<GPR:MODE>")])

(define_mode_iterator JOIN_MODE [HI
                                 SI
                                 (DI "TARGET_64BIT")
                                 (SF "TARGET_HARD_FLOAT")
                                 (DF "TARGET_DOUBLE_FLOAT")])

;; 2 HI/SI/DI/SF/DF loads are joined.
;; Daimyo does not support bonding of two LBs, hence QI mode is not included.
;; The loads must be non-volatile as they might be reordered at the time of asm
;; generation.
(define_peephole2
  [(set (match_operand:JOIN_MODE 0 "register_operand")
        (match_operand:JOIN_MODE 1 "non_volatile_mem_operand"))
   (set (match_operand:JOIN_MODE 2 "register_operand")
        (match_operand:JOIN_MODE 3 "non_volatile_mem_operand"))]
  "ENABLE_LD_ST_PAIRS
   && riscv_load_store_bonding_p (operands, <JOIN_MODE:MODE>mode, true)"
  [(parallel [(set (match_dup 0)
                   (match_dup 1))
              (set (match_dup 2)
                   (match_dup 3))])]
  "")

;; 2 HI/SI/DI/SF/DF stores are joined.
;; Daimyo does not support bonding of two SBs, hence QI mode is not included.
(define_peephole2
  [(set (match_operand:JOIN_MODE 0 "memory_operand")
        (match_operand:JOIN_MODE 1 "register_operand"))
   (set (match_operand:JOIN_MODE 2 "memory_operand")
        (match_operand:JOIN_MODE 3 "register_operand"))]
  "ENABLE_LD_ST_PAIRS
   && riscv_load_store_bonding_p (operands, <JOIN_MODE:MODE>mode, false)"
  [(parallel [(set (match_dup 0)
                   (match_dup 1))
              (set (match_dup 2)
                   (match_dup 3))])]
  "")

;; 2 HI loads are joined.
(define_peephole2
  [(set (match_operand:SI 0 "register_operand")
        (any_extend:SI (match_operand:HI 1 "non_volatile_mem_operand")))
   (set (match_operand:SI 2 "register_operand")
        (any_extend:SI (match_operand:HI 3 "non_volatile_mem_operand")))]
  "ENABLE_LD_ST_PAIRS
   && riscv_load_store_bonding_p (operands, HImode, true)"
  [(parallel [(set (match_dup 0)
                   (any_extend:SI (match_dup 1)))
              (set (match_dup 2)
                   (any_extend:SI (match_dup 3)))])]
  "")

;; 2 HI loads are joined.
(define_peephole2
  [(set (match_operand:DI 0 "register_operand")
        (any_extend:DI (match_operand:HI 1 "non_volatile_mem_operand")))
   (set (match_operand:DI 2 "register_operand")
        (any_extend:DI (match_operand:HI 3 "non_volatile_mem_operand")))]
  "ENABLE_LD_ST_PAIRS && TARGET_64BIT
   && riscv_load_store_bonding_p (operands, HImode, true)"
  [(parallel [(set (match_dup 0)
                   (any_extend:DI (match_dup 1)))
              (set (match_dup 2)
                   (any_extend:DI (match_dup 3)))])]
  "")

;; 2 SI loads are joined.
(define_peephole2
  [(set (match_operand:DI 0 "register_operand")
        (any_extend:DI (match_operand:SI 1 "non_volatile_mem_operand")))
   (set (match_operand:DI 2 "register_operand")
        (any_extend:DI (match_operand:SI 3 "non_volatile_mem_operand")))]
  "ENABLE_LD_ST_PAIRS && TARGET_64BIT
   && riscv_load_store_bonding_p (operands, SImode, true)"
  [(parallel [(set (match_dup 0)
                   (any_extend:DI (match_dup 1)))
              (set (match_dup 2)
                   (any_extend:DI (match_dup 3)))])]
  "")

;; Match paired HI/SI/DI/SF/DFmode loads
(define_insn "*join2_load<JOIN_MODE:mode>"
  [(set (match_operand:JOIN_MODE 0 "register_operand" "=r,f")
        (match_operand:JOIN_MODE 1 "memory_operand" "m,m"))
   (set (match_operand:JOIN_MODE 2 "register_operand" "=r,f")
        (match_operand:JOIN_MODE 3 "memory_operand" "m,ms"))]
  "ENABLE_LD_ST_PAIRS && reload_completed"
  {
    bool load_p = true;
    return riscv_output_join2_insns (operands, <JOIN_MODE:MODE>mode, load_p,
                                     false);
  }
  [(set_attr "move_type" "load,fpload")
   (set_attr "mode" "<JOIN_MODE:MODE>")])

;; Match paired HI/SI/DI/SF/DFmode stores.
(define_insn "*join2_store<JOIN_MODE:mode>"
  [(set (match_operand:JOIN_MODE 0 "memory_operand" "=m,m")
        (match_operand:JOIN_MODE 1 "register_operand" "r,f"))
   (set (match_operand:JOIN_MODE 2 "memory_operand" "=m,m")
        (match_operand:JOIN_MODE 3 "register_operand" "r,f"))]
  "ENABLE_LD_ST_PAIRS && reload_completed"
  {
    bool load_p = false;
    return riscv_output_join2_insns (operands, <JOIN_MODE:MODE>mode, load_p,
                                     false);
  }
  [(set_attr "move_type" "store,fpstore")
   (set_attr "mode" "<JOIN_MODE:MODE>")])

;; Match paired HImode loads.
(define_insn "*join2_loadsihi"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (any_extend:SI (match_operand:HI 1 "non_volatile_mem_operand" "m")))
   (set (match_operand:SI 2 "register_operand" "=r")
        (any_extend:SI (match_operand:HI 3 "non_volatile_mem_operand" "m")))]
  "ENABLE_LD_ST_PAIRS && reload_completed"
  {
    if (!reg_overlap_mentioned_p (operands[0], operands[3]))
    {
      output_asm_insn ("lh<u>\t%0,%1", operands);
      output_asm_insn ("lh<u>\t%2,%3", operands);
    }
    else
    {
      /* Make sure operands[2] and operands[1] don't overlap.  */
      gcc_assert(!reg_overlap_mentioned_p (operands[2], operands[1]));
      output_asm_insn ("lh<u>\t%2,%3", operands);
      output_asm_insn ("lh<u>\t%0,%1", operands);
    }
    return "";
  }
  [(set_attr "move_type" "load")
   (set_attr "mode" "HI")])

;; Match paired HImode loads.
(define_insn "*join2_loaddihi"
  [(set (match_operand:DI 0 "register_operand" "=r")
        (any_extend:DI (match_operand:HI 1 "non_volatile_mem_operand" "m")))
   (set (match_operand:DI 2 "register_operand" "=r")
        (any_extend:DI (match_operand:HI 3 "non_volatile_mem_operand" "m")))]
  "ENABLE_LD_ST_PAIRS && TARGET_64BIT && reload_completed"
  {
    if (!reg_overlap_mentioned_p (operands[0], operands[3]))
    {
      output_asm_insn ("lh<u>\t%0,%1", operands);
      output_asm_insn ("lh<u>\t%2,%3", operands);
    }
    else
    {
      /* Make sure operands[2] and operands[1] don't overlap.  */
      gcc_assert(!reg_overlap_mentioned_p (operands[2], operands[1]));
      output_asm_insn ("lh<u>\t%2,%3", operands);
      output_asm_insn ("lh<u>\t%0,%1", operands);
    }
    return "";
  }
  [(set_attr "move_type" "load")
   (set_attr "mode" "HI")])

;; Match paired SImode loads.
(define_insn "*join2_loaddisi_zero_extend"
  [(set (match_operand:DI 0 "register_operand" "=r")
        (zero_extend:DI (match_operand:SI 1 "non_volatile_mem_operand" "m")))
   (set (match_operand:DI 2 "register_operand" "=r")
        (zero_extend:DI (match_operand:SI 3 "non_volatile_mem_operand" "m")))]
  "ENABLE_LD_ST_PAIRS && TARGET_64BIT && reload_completed"
  {
    if (!reg_overlap_mentioned_p (operands[0], operands[3]))
    {
      output_asm_insn ("lwu\t%0,%1", operands);
      output_asm_insn ("lwu\t%2,%3", operands);
    }
    else
    {
      /* Make sure operands[2] and operands[1] don't overlap.  */
      gcc_assert(!reg_overlap_mentioned_p (operands[2], operands[1]));
      output_asm_insn ("lwu\t%2,%3", operands);
      output_asm_insn ("lwu\t%0,%1", operands);
    }
    return "";
  }
  [(set_attr "move_type" "load")
   (set_attr "mode" "SI")])

;; Match paired SImode loads.
(define_insn "*join2_loaddisi_sign_extend"
  [(set (match_operand:DI 0 "register_operand" "=r")
        (sign_extend:DI (match_operand:SI 1 "non_volatile_mem_operand" "m")))
   (set (match_operand:DI 2 "register_operand" "=r")
        (sign_extend:DI (match_operand:SI 3 "non_volatile_mem_operand" "m")))]
  "ENABLE_LD_ST_PAIRS && TARGET_64BIT && reload_completed"
  {
    return riscv_output_join2_insns (operands, SImode, true, true);
  }
  [(set_attr "move_type" "load")
   (set_attr "mode" "SI")])

(define_insn "hazard_nop"
  [(unspec_volatile [(const_int 0)] UNSPECV_HAZARD_NOP)]
  ""
  "# branch to branch hazard"
  [(set_attr "type" "ghost")
   (set_attr "mode" "none")])

;; Combine lh and right shift 8 to lb.
(define_peephole2
  [(set (match_operand:SI 0 "register_operand")
       (sign_extend:SI (mem:HI (match_operand 1 "nonimmediate_operand"))))
   (set (match_operand:DI 2 "register_operand")
       (sign_extend:DI (ashiftrt:SI (match_dup 0) (const_int 8))))]
  "TARGET_64BIT && !BYTES_BIG_ENDIAN
   && (REGNO (operands[0]) == REGNO (operands[2])
       || peep2_reg_dead_p (0, operands[0]))"
  [(set (match_operand:DI 2 "register_operand")
       (sign_extend:DI (mem:QI (plus:DI (match_operand 1
                                         "nonimmediate_operand")
                                        (const_int 1)))))]
  "")

;; flti instruction
(define_insn "riscv_mips_flti"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec [(match_operand:SI 1 "flti_operand" "zlti")]
		   UNSPEC_FLTI))]
  "TARGET_XMIPSTRIG"
  "mips.flti.s %0,%1"
  [(set_attr "type" "fsin")]
)

;; xmipstrig instructions
(define_expand "sinsf2"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_SIN))]
  "TARGET_XMIPSTRIG"
  {
    rtx scalereg = riscv_get_trig_scale_value(FLTI_CONST_1_BY_2PI);
    emit_insn (gen_mulsf3(scalereg, operands[1], scalereg));
    emit_insn (gen_riscv_mips_fsinhz (operands[0], scalereg));
    DONE;
  }
)

(define_expand "riscv_mips_asinf_hz"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_ASIN))]
  "TARGET_XMIPSTRIG"
  {
    riscv_output_asinf_hz (operands[0], operands[1]);
    DONE;
  }
)

(define_expand "asinsf2"
  [(set (match_operand:SF 0 "register_operand" "=f")
        (unspec:SF [(match_operand:SF 1 "register_operand" "f")]
                   UNSPEC_ASIN))]
  "TARGET_XMIPSTRIG"
  {
    rtx temp = riscv_output_asinf_hz (operands[0], operands[1]);
    rtx scalereg = riscv_get_trig_scale_value(FLTI_CONST_2PI);
    emit_insn (gen_mulsf3(operands[0], temp, scalereg));
    DONE;
  }
)

(define_expand "riscv_mips_acosf_hz"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_ACOS))]
  "TARGET_XMIPSTRIG"
  {
    rtx temp = riscv_output_asinf_hz (operands[0], operands[1]);
    rtx const_0_25 = riscv_get_trig_const(FLI_CONST_0_25_INDEX);
    rtx temp_reg1 = gen_reg_rtx (SFmode);
    emit_move_insn(temp_reg1, const_0_25);
    emit_insn (gen_subsf3(operands[0], temp_reg1, temp));
    DONE;
  }
)

(define_expand "acossf2"
  [(set (match_operand:SF 0 "register_operand" "=f")
        (unspec:SF [(match_operand:SF 1 "register_operand" "f")]
                   UNSPEC_ACOS))]
  "TARGET_XMIPSTRIG"
  {
    emit_insn (gen_riscv_mips_acosf_hz (operands[0], operands[1]));
    rtx scalereg = riscv_get_trig_scale_value(FLTI_CONST_2PI);
    emit_insn (gen_mulsf3(operands[0], operands[0], scalereg));
    DONE;
  }
)

(define_insn "riscv_mips_fsinhz"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_SIN))]
  "TARGET_XMIPSTRIG"
  "mips.fsinhz.s %0,%1"
  [(set_attr "type" "fsin")]
)

(define_expand "cossf2"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_COS))]
  "TARGET_XMIPSTRIG"
  {
    rtx scalereg = riscv_get_trig_scale_value(FLTI_CONST_1_BY_2PI);
    emit_insn (gen_mulsf3(scalereg, operands[1], scalereg));
    emit_insn (gen_riscv_mips_fcoshz (operands[0], scalereg));
    DONE;
  }
)

(define_insn "riscv_mips_fcoshz"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_COS))]
  "TARGET_XMIPSTRIG"
  "mips.fcoshz.s %0,%1"
  [(set_attr "type" "fcos")]
)

(define_expand "tansf2"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_TAN))]
  "TARGET_XMIPSTRIG"
  {
    rtx scalereg = riscv_get_trig_scale_value(FLTI_CONST_1_BY_2PI);
    emit_insn (gen_mulsf3(scalereg, operands[1], scalereg));
    emit_insn (gen_riscv_mips_ftanhz (operands[0], scalereg));
    DONE;
  }
)

(define_insn "riscv_mips_ftanhz"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_TAN))]
  "TARGET_XMIPSTRIG"
  "mips.ftanhz.s %0,%1"
  [(set_attr "type" "ftan")]
)

(define_expand "atansf2"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_ATAN))]
  "TARGET_XMIPSTRIG"
  {
    rtx scalereg = riscv_get_trig_scale_value(FLTI_CONST_2PI);
    emit_insn (gen_riscv_mips_fatanhz (operands[0], operands[1]));
    emit_insn (gen_mulsf3(operands[0], operands[0], scalereg));
    DONE;
  }
)

(define_insn "riscv_mips_fatanhz"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_ATAN))]
  "TARGET_XMIPSTRIG"
  "mips.fatanhz.s %0,%1"
  [(set_attr "type" "fatan")]
)

(define_insn "riscv_mips_fversinhz"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_VERSIN))]
  "TARGET_XMIPSTRIG"
  "mips.fversinhz.s %0,%1"
  [(set_attr "type" "fversin")]
)

(define_insn "riscv_mips_ffrecip"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_FRECIP))]
  "TARGET_XMIPSTRIG"
  "mips.ffrecip.s %0,%1"
  [(set_attr "type" "ffrecip")]
)

(define_insn "riscv_mips_ffrsqrt"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_FRECIPSQRT))]
  "TARGET_XMIPSTRIG"
  {
      return "mips.ffrsqrt.s %0,%1";
  }
  [(set_attr "type" "ffrecipsqrt")]
)

(define_expand "tanhsf2"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_TANH))]
  "TARGET_XMIPSTRIG"
  {
    emit_insn (gen_riscv_mips_fftanh (operands[0], operands[1]));
    DONE;
  }
)

(define_insn "riscv_mips_fftanh"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_TANH))]
  "TARGET_XMIPSTRIG"
  "mips.fftanh.s %0,%1"
  [(set_attr "type" "ftanh")]
)

(define_expand "log2sf2"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_LOG2F))]
  "TARGET_XMIPSTRIG"
  {
    emit_insn (gen_riscv_mips_fflog2 (operands[0], operands[1]));
    DONE;
  }
)

(define_expand "logsf2"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_LOGF))]
  "TARGET_XMIPSTRIG"
  {
    rtx scalereg = riscv_get_trig_scale_value(FLTI_CONST_LN_OF_2);
    emit_insn (gen_riscv_mips_fflog2 (operands[0], operands[1]));
    emit_insn (gen_mulsf3(operands[0], operands[0], scalereg));
    DONE;
  }
)

(define_expand "log10sf2"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_LOG10F))]
  "TARGET_XMIPSTRIG"
  {
    rtx scalereg = riscv_get_trig_scale_value(FLTI_CONST_LOG10_OF_2);
    emit_insn (gen_riscv_mips_fflog2 (operands[0], operands[1]));
    emit_insn (gen_mulsf3(operands[0], operands[0], scalereg));
    DONE;
  }
)

(define_expand "riscv_mips_logf"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_LOGF))]
  "TARGET_XMIPSTRIG"
  {
    rtx scalereg = riscv_get_trig_scale_value(FLTI_CONST_LN_OF_2);
    emit_insn (gen_riscv_mips_fflog2 (operands[0], operands[1]));
    emit_insn (gen_mulsf3(operands[0], operands[0], scalereg));
    DONE;
  }
)

(define_expand "riscv_mips_log10f"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_LOG10F))]
  "TARGET_XMIPSTRIG"
  {
    rtx scalereg = riscv_get_trig_scale_value(FLTI_CONST_LOG10_OF_2);
    emit_insn (gen_riscv_mips_fflog2 (operands[0], operands[1]));
    emit_insn (gen_mulsf3(operands[0], operands[0], scalereg));
    DONE;
  }
)

(define_insn "riscv_mips_fflog2"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_LOG2F))]
  "TARGET_XMIPSTRIG"
  "mips.fflog2.s %0,%1"
  [(set_attr "type" "flog")]
)

(define_expand "expsf2"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_EXPF))]
  "TARGET_XMIPSTRIG"
  {
    rtx scalereg = riscv_get_trig_scale_value(FLTI_CONST_LOG2_OF_E);
    emit_insn (gen_mulsf3(scalereg, operands[1], scalereg));
    emit_insn (gen_riscv_mips_ffexp2 (operands[0], scalereg));

    DONE;
  }
)

(define_expand "exp10sf2"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_EXP10F))]
  "TARGET_XMIPSTRIG"
  {
    rtx scalereg = riscv_get_trig_scale_value(FLTI_CONST_LOG2_OF_10);
    emit_insn (gen_mulsf3(scalereg, operands[1], scalereg));
    emit_insn (gen_riscv_mips_ffexp2 (operands[0], scalereg));

    DONE;
  }
)

(define_expand "riscv_mips_exp10f"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_EXP10F))]
  "TARGET_XMIPSTRIG"
  {
    rtx scalereg = riscv_get_trig_scale_value(FLTI_CONST_LOG2_OF_10);
    emit_insn (gen_mulsf3(scalereg, operands[1], scalereg));
    emit_insn (gen_riscv_mips_ffexp2 (operands[0], scalereg));

    DONE;
  }
)

(define_insn "riscv_mips_ffexp2"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_EXPF))]
  "TARGET_XMIPSTRIG"
  "mips.ffexp2.s %0,%1"
  [(set_attr "type" "fexp")]
)

(define_insn "riscv_mips_ffsqrt"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_FFSQRT))]
  "TARGET_XMIPSTRIG"
  "mips.ffsqrt.s %0,%1"
  [(set_attr "type" "ffsqrt")]
)

(define_expand "hypotsf3"
  [(set (match_operand:SF 0 "register_operand" "=f")
        (unspec:SF [(match_operand:SF 1 "register_operand" "f")
              (match_operand:SF 2 "register_operand" "f")]
                   UNSPEC_HYPOTF))]
  "TARGET_XMIPSTRIG"
  {
    rtx temp_reg1 = gen_reg_rtx (SFmode);
    rtx temp_reg2 = gen_reg_rtx (SFmode);

    emit_insn (gen_mulsf3(temp_reg1, operands[1], operands[1]));
    emit_insn (gen_mulsf3(temp_reg2, operands[2], operands[2]));
    emit_insn (gen_addsf3(temp_reg2, temp_reg2, temp_reg1));
    emit_insn (gen_riscv_mips_ffsqrt (operands[0], temp_reg2));

    DONE;
  }
)

(define_expand "riscv_mips_hypotf"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")
              (match_operand:SF 2 "register_operand" "f")]
		   UNSPEC_HYPOTF))]
  "TARGET_XMIPSTRIG"
  {
    rtx temp_reg1 = gen_reg_rtx (SFmode);
    rtx temp_reg2 = gen_reg_rtx (SFmode);

    emit_insn (gen_mulsf3(temp_reg1, operands[1], operands[1]));
    emit_insn (gen_mulsf3(temp_reg2, operands[2], operands[2]));
    emit_insn (gen_addsf3(temp_reg2, temp_reg2, temp_reg1));
    emit_insn (gen_riscv_mips_ffsqrt (operands[0], temp_reg2));

    DONE;
  }
)

(define_expand "riscv_mips_atan2f_hz"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")
              (match_operand:SF 2 "register_operand" "f")]
		   UNSPEC_ATAN2F_HZ))]
  "TARGET_XMIPSTRIG"
  {
    rtx temp_reg1 = gen_reg_rtx (SFmode);

    emit_insn (gen_divsf3(temp_reg1, operands[1], operands[2]));
    emit_insn (gen_riscv_mips_fatanhz (operands[0], temp_reg1));

    DONE;
  }
)

(define_expand "atan2sf3"
  [(set (match_operand:SF 0 "register_operand" "=f")
        (unspec:SF [(match_operand:SF 1 "register_operand" "f")
              (match_operand:SF 2 "register_operand" "f")]
                   UNSPEC_ATAN2F))]
  "TARGET_XMIPSTRIG"
  {
    rtx temp_reg1 = gen_reg_rtx (SFmode);

    emit_insn (gen_divsf3(temp_reg1, operands[1], operands[2]));
    emit_insn (gen_riscv_mips_fatanhz (operands[0], temp_reg1));

    rtx scalereg = riscv_get_trig_scale_value(FLTI_CONST_2PI);
    emit_insn (gen_mulsf3(operands[0], operands[0], scalereg));
    DONE;
  }
)

(define_expand "riscv_mips_atan2f"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")
              (match_operand:SF 2 "register_operand" "f")]
		   UNSPEC_ATAN2F))]
  "TARGET_XMIPSTRIG"
  {
    rtx temp_reg1 = gen_reg_rtx (SFmode);

    emit_insn (gen_divsf3(temp_reg1, operands[1], operands[2]));
    emit_insn (gen_riscv_mips_fatanhz (operands[0], temp_reg1));

    rtx scalereg = riscv_get_trig_scale_value(FLTI_CONST_2PI);
    emit_insn (gen_mulsf3(operands[0], operands[0], scalereg));
    DONE;
  }
)

(define_expand "riscv_mips_tanf_hz_fc"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_TANF_HZ_FC))]
  "TARGET_XMIPSTRIG"
  {
    rtx const_0_25 = riscv_get_trig_const(FLI_CONST_0_25_INDEX);
    rtx temp_reg1 = gen_reg_rtx (SFmode);
    emit_move_insn(temp_reg1, const_0_25);
    emit_insn (gen_subsf3(temp_reg1, temp_reg1, operands[1]));
    emit_insn (gen_riscv_mips_ftanhz (operands[0], operands[1]));
    emit_insn (gen_riscv_mips_ftanhz (temp_reg1, temp_reg1));
    emit_insn (gen_riscv_mips_ffrecip (temp_reg1, temp_reg1));
    emit_insn (gen_fmaxsf3(operands[0], operands[0], temp_reg1));
    DONE;
  }
)

(define_expand "riscv_mips_atanf_hz_fc"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(unspec:SF [(match_operand:SF 1 "register_operand" "f")]
		   UNSPEC_ATANF_HZ_FC))]
  "TARGET_XMIPSTRIG"
  {
    rtx temp_reg0 = gen_reg_rtx (SFmode);
    emit_insn (gen_riscv_mips_ffrecip (temp_reg0, operands[1]));
    rtx const_0_25 = riscv_get_trig_const(FLI_CONST_0_25_INDEX);
    rtx temp_reg1 = gen_reg_rtx (SFmode);
    emit_move_insn(temp_reg1, const_0_25);
    emit_insn (gen_riscv_mips_fatanhz (temp_reg0, temp_reg0));
    emit_insn (gen_subsf3(temp_reg0, temp_reg1, temp_reg0));
    emit_insn (gen_riscv_mips_fatanhz (operands[0], operands[1]));
    emit_insn (gen_fmaxsf3(operands[0], operands[0], temp_reg0));
    DONE;
  }
)
