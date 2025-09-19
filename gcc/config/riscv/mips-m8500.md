;; DFA-based pipeline description for m8500.
;;
;; Copyright (C) 2018-2024 Free Software Foundation, Inc.
;;
;; This file is part of GCC.
;;
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

;; MIPS m8500 has 7 pipelines
;; ALU0 pipe   - ALU, Branch
;; ALU1 pipe   - ALU, Branch
;; MDU pipe    - Integer MUL and Integer Div
;; SFPU pipe   - Short FPU
;; LFPU pipe   - Long FPU
;; TRIG pipe   - Trig
;; LSU pipe    - LSU pipe

(define_automaton "mips_m8500_alu_pipe, mips_m8500_mdu_pipe, mips_m8500_fpu_pipe, mips_m8500_trig_pipe")

(define_cpu_unit "mips_m8500_alu0, mips_m8500_alu1, mips_m8500_lsu"
		 "mips_m8500_alu_pipe")

(define_cpu_unit "mips_m8500_imul, mips_m8500_idiv" "mips_m8500_mdu_pipe")

(define_cpu_unit "mips_m8500_fpu_short, mips_m8500_fpu_long" "mips_m8500_fpu_pipe")

(define_cpu_unit "mips_m8500_trig" "mips_m8500_trig_pipe")

;;
;; FPU pipe
;;

(define_insn_reservation "mips_m8500_fpu_fadd" 4
  (and (eq_attr "tune" "mips_m8500")
       (eq_attr "type" "fadd"))
  "mips_m8500_fpu_long")

(define_insn_reservation "mips_m8500_fpu_move" 1
  (and (eq_attr "tune" "mips_m8500")
       (eq_attr "type" "fmove"))
  "mips_m8500_fpu_short")

(define_insn_reservation "mips_m8500_fpu_fabs" 2
  (and (eq_attr "tune" "mips_m8500")
       (eq_attr "type" "fcmp"))
  "mips_m8500_fpu_short")

(define_insn_reservation "mips_m8500_fpu_fload" 8
  (and (eq_attr "tune" "mips_m8500")
       (eq_attr "type" "fpload"))
  "mips_m8500_fpu_long")

(define_insn_reservation "mips_m8500_fpu_fstore" 1
  (and (eq_attr "tune" "mips_m8500")
       (eq_attr "type" "fpstore"))
  "mips_m8500_fpu_long")

(define_insn_reservation "mips_m8500_fpu_fmadd" 8
  (and (eq_attr "tune" "mips_m8500")
       (eq_attr "type" "fmadd"))
  "mips_m8500_fpu_long")

(define_insn_reservation "mips_m8500_fpu_fmul" 4
  (and (eq_attr "tune" "mips_m8500")
       (eq_attr "type" "fmul"))
  "mips_m8500_fpu_long")

(define_insn_reservation "mips_m8500_fpu_div" 10
  (and (eq_attr "tune" "mips_m8500")
       (eq_attr "type" "fdiv,fsqrt"))
  "mips_m8500_fpu_long*10")

(define_insn_reservation "mips_m8500_fpu_fcvt" 2
  (and (eq_attr "tune" "mips_m8500")
       (eq_attr "type" "fcvt,fcvt_i2f,fcvt_f2i"))
  "mips_m8500_fpu_long")

(define_insn_reservation "mips_m8500_fpu_fmtc" 7
  (and (eq_attr "tune" "mips_m8500")
       (eq_attr "type" "mtc"))
  "mips_m8500_fpu_long")

(define_insn_reservation "mips_m8500_fpu_fmfc" 7
  (and (eq_attr "tune" "mips_m8500")
       (eq_attr "type" "mfc"))
  "mips_m8500_fpu_long")

;;
;; Integer pipe
;;

(define_insn_reservation "mips_m8500_int_load" 4
  (and (eq_attr "tune" "mips_m8500")
       (eq_attr "type" "load"))
  "mips_m8500_lsu")

(define_insn_reservation "mips_m8500_int_store" 3
  (and (eq_attr "tune" "mips_m8500")
       (eq_attr "type" "store"))
  "mips_m8500_lsu")

(define_insn_reservation "mips_m8500_int_arith_1" 1
  (and (eq_attr "tune" "mips_m8500")
       (eq_attr "type" "unknown,const,arith,shift,slt,multi,auipc,logical,move,bitmanip,min,max,minu,maxu,clz,ctz,rotate,atomic,condmove,crypto,mvpair,zicond"))
  "mips_m8500_alu0 | mips_m8500_alu1")

(define_insn_reservation "mips_m8500_int_nop" 0
  (and (eq_attr "tune" "mips_m8500")
       (eq_attr "type" "nop"))
  "mips_m8500_alu0 | mips_m8500_alu1")

(define_insn_reservation "mips_m8500_dsp_mult" 4
  (and (eq_attr "tune" "mips_m8500")
       (eq_attr "type" "imul,cpop,clmul"))
  "mips_m8500_imul")

(define_insn_reservation "mips_m8500_int_div" 8
  (and (eq_attr "tune" "mips_m8500")
       (eq_attr "type" "idiv"))
  "mips_m8500_idiv*5")

(define_insn_reservation "mips_m8500_int_branch" 1
  (and (eq_attr "tune" "mips_m8500")
       (eq_attr "type" "branch,jump,ret,sfb_alu,trap"))
  "mips_m8500_alu0")

(define_insn_reservation "mips_m8500_int_call" 2
  (and (eq_attr "tune" "mips_m8500")
       (eq_attr "type" "call,jalr"))
  "mips_m8500_alu0")

;;
;; TRIG
;; TODO: untill we get final latency numbers for trig instructions from the hw spec, we assume it as 10 cycles
;;
(define_insn_reservation "mips_m8500_trig" 10
  (and (eq_attr "tune" "mips_m8500")
       (eq_attr "type" "fsin, fcos, ftan, fversin, ffrecip, ffrecipsqrt, fatan, ftanh, flog, fexp, ffsqrt"))
  "mips_m8500_trig")
