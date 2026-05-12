;; DFA-based pipeline description for RISC-V Daimyo targets.
;; Copyright (C) 2011-2024 Free Software Foundation, Inc.
;; Contributed by Chao-ying Fu (cfu@wavecomp.com).
;; Based on MIPS target for GNU compiler.

;; This file is part of GCC.

;; GCC is free software; you can redistribute it and/or modify it
;; under the terms of the GNU General Public License as published
;; by the Free Software Foundation; either version 3, or (at your
;; option) any later version.

;; GCC is distributed in the hope that it will be useful, but WITHOUT
;; ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
;; or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
;; License for more details.

;; You should have received a copy of the GNU General Public License
;; along with GCC; see the file COPYING3.  If not see
;; <http://www.gnu.org/licenses/>.


(define_automaton "mips_daimyo_int_pipe, mips_daimyo_mdu_pipe, mips_daimyo_fpu_short_pipe,
     mips_daimyo_fpu_long_pipe")

(define_cpu_unit "mips_daimyo_gpmul, mips_daimyo_gpdiv" "mips_daimyo_mdu_pipe")
(define_cpu_unit "mips_daimyo_agen, mips_daimyo_alu1, mips_daimyo_lsu" "mips_daimyo_int_pipe")
(define_cpu_unit "mips_daimyo_control, mips_daimyo_ctu, mips_daimyo_alu0" "mips_daimyo_int_pipe")

(define_cpu_unit "mips_daimyo_fpu_short" "mips_daimyo_fpu_short_pipe")
(define_cpu_unit "mips_daimyo_fpu_long"	"mips_daimyo_fpu_long_pipe")

(define_reservation "mips_daimyo_control_ctu" "mips_daimyo_control, mips_daimyo_ctu")
(define_reservation "mips_daimyo_control_alu0" "mips_daimyo_control, mips_daimyo_alu0")
(define_reservation "mips_daimyo_agen_lsu" "mips_daimyo_agen, mips_daimyo_lsu")
(define_reservation "mips_daimyo_agen_alu1" "mips_daimyo_agen, mips_daimyo_alu1")

(define_insn_reservation "mips_daimyo_alu" 1
  (and (ior (eq_attr "tune" "mips_daimyo") (eq_attr "tune" "mips_i8500"))
       (eq_attr "type" "unknown,const,arith,shift,slt,multi,auipc,logical,move,
		bitmanip,min,max,minu,maxu,clz,ctz,rotate,atomic,crypto,mvpair,
		zicond,clmul"))
  "mips_daimyo_control_alu0 | mips_daimyo_agen_alu1")

(define_insn_reservation "mips_daimyo_cmove" 2
  (and (ior (eq_attr "tune" "mips_daimyo") (eq_attr "tune" "mips_i8500"))
       (eq_attr "type" "condmove"))
  "mips_daimyo_control_alu0 | mips_daimyo_agen_alu1")

(define_insn_reservation "mips_daimyo_nop" 0
  (and (ior (eq_attr "tune" "mips_daimyo") (eq_attr "tune" "mips_i8500"))
       (eq_attr "type" "nop"))
  "nothing")

;; mulw takes 3 cycles where as remaining instructions take 4.
(define_insn_reservation "mips_daimyo_imul" 4
  (and (ior (eq_attr "tune" "mips_daimyo") (eq_attr "tune" "mips_i8500"))
       (eq_attr "type" "imul"))
  "mips_daimyo_gpmul")

(define_insn_reservation "mips_daimyo_cpop" 3
  (and (ior (eq_attr "tune" "mips_daimyo") (eq_attr "tune" "mips_i8500"))
       (eq_attr "type" "cpop"))
  "mips_daimyo_gpmul")

(define_insn_reservation "mips_daimyo_idiv" 22
  (and (ior (eq_attr "tune" "mips_daimyo") (eq_attr "tune" "mips_i8500"))
       (eq_attr "type" "idiv"))
  "mips_daimyo_gpdiv*22")

(define_insn_reservation "mips_daimyo_load" 3
  (and (ior (eq_attr "tune" "mips_daimyo") (eq_attr "tune" "mips_i8500"))
       (eq_attr "type" "load,fpload"))
  "mips_daimyo_agen_lsu")

(define_insn_reservation "mips_daimyo_store" 1
  (and (ior (eq_attr "tune" "mips_daimyo") (eq_attr "tune" "mips_i8500"))
       (eq_attr "type" "store,fpstore"))
  "mips_daimyo_agen_lsu")

(define_insn_reservation "mips_daimyo_branch" 1
  (and (ior (eq_attr "tune" "mips_daimyo") (eq_attr "tune" "mips_i8500"))
       (eq_attr "type" "branch,jump,call,jalr,ret,sfb_alu,trap"))
  "mips_daimyo_control_ctu")

(define_insn_reservation "mips_daimyo_xfer" 1
  (and (ior (eq_attr "tune" "mips_daimyo") (eq_attr "tune" "mips_i8500"))
       (eq_attr "type" "mfc,mtc"))
  "mips_daimyo_control_alu0 | mips_daimyo_agen_alu1")

;; fmin/fmax takes 2 cycles where as remaining fmove instructions such as
;; fabs, fsgnj, fmv.w.x takes 1 cycle. so define the latency for fmove as
;; 2 (worstcase)
(define_insn_reservation "mips_daimyo_fmove" 2
  (and (ior (eq_attr "tune" "mips_daimyo") (eq_attr "tune" "mips_i8500"))
       (eq_attr "type" "fmove"))
  "mips_daimyo_fpu_short")

(define_insn_reservation "mips_daimyo_fcmp" 2
  (and (ior (eq_attr "tune" "mips_daimyo") (eq_attr "tune" "mips_i8500"))
       (eq_attr "type" "fcmp"))
  "mips_daimyo_fpu_short")

(define_insn_reservation "mips_daimyo_fcvt" 4
  (and (ior (eq_attr "tune" "mips_daimyo") (eq_attr "tune" "mips_i8500"))
       (eq_attr "type" "fadd,fcvt,fcvt_i2f,fcvt_f2i"))
  "mips_daimyo_fpu_long")

(define_insn_reservation "mips_daimyo_fmul" 5
  (and (ior (eq_attr "tune" "mips_daimyo") (eq_attr "tune" "mips_i8500"))
       (eq_attr "type" "fmul"))
  "mips_daimyo_fpu_long")

(define_insn_reservation "mips_daimyo_fmadd" 8
  (and (ior (eq_attr "tune" "mips_daimyo") (eq_attr "tune" "mips_i8500"))
       (eq_attr "type" "fmadd"))
  "mips_daimyo_fpu_long")

(define_insn_reservation "mips_daimyo_fdiv_sf" 24
  (and (ior (eq_attr "tune" "mips_daimyo") (eq_attr "tune" "mips_i8500"))
       (and (eq_attr "type" "fdiv,fsqrt")
            (eq_attr "mode" "SF")))
  "mips_daimyo_fpu_long*24")

(define_insn_reservation "mips_daimyo_fdiv_df" 32
  (and (ior (eq_attr "tune" "mips_daimyo") (eq_attr "tune" "mips_i8500"))
       (and (eq_attr "type" "fdiv")
			(eq_attr "mode" "DF")))
  "mips_daimyo_fpu_long*32")

(define_insn_reservation "mips_daimyo_fsqrt_df" 36
  (and (ior (eq_attr "tune" "mips_daimyo") (eq_attr "tune" "mips_i8500"))
       (and (eq_attr "type" "fsqrt")
            (eq_attr "mode" "DF")))
  "mips_daimyo_fpu_long*36")
