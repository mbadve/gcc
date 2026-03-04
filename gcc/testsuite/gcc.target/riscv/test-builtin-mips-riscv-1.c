/* { dg-do compile } */
/* { dg-options "-march=rv32imafd_xmipstrig_zfa -mtune=mips-m8500 -mabi=ilp32d" } */
/* { dg-skip-if "" { *-*-* } { "-O0" "-O2" "-Os" "-Oz" "-g" "-flto" } } */
/* { dg-final { check-function-bodies "**" "" } } */

/*
**test_builtin1:
**  mips.fsinhz.s fa0,fa0
**  ret
*/
float test_builtin1(float x) {
  return __builtin_riscv_mips_fsinhz(x);
}

/*
**test_builtin2:
**  mips.fcoshz.s fa0,fa0
**  ret
*/
float test_builtin2(float x) {
  return __builtin_riscv_mips_fcoshz(x);
}

/*
**test_builtin3:
**  mips.ftanhz.s fa0,fa0
**  ret
*/
float test_builtin3(float x) {
  return __builtin_riscv_mips_ftanhz(x);
}

/*
**test_builtin4:
**  mips.fversinhz.s fa0,fa0
**  ret
*/
float test_builtin4(float x) {
  return __builtin_riscv_mips_fversinhz(x);
}

/*
**test_builtin5:
**  mips.ffrecip.s fa0,fa0
**  ret
*/
float test_builtin5(float x) {
  return __builtin_riscv_mips_ffrecip(x);
}

/*
**test_builtin6:
**  mips.ffrsqrt.s fa0,fa0
**  ret
*/
float test_builtin6(float x) {
  return __builtin_riscv_mips_ffrsqrt(x);
}

/*
**test_builtin7:
**  mips.fatanhz.s fa0,fa0
**  ret
*/
float test_builtin7(float x) {
  return __builtin_riscv_mips_fatanhz(x);
}

/*
**test_builtin9:
**  mips.ffexp2.s fa0,fa0
**  ret
*/
float test_builtin9(float x) {
  return __builtin_riscv_mips_ffexp2(x);
}

/*
**test_builtin10:
**  mips.ffsqrt.s fa0,fa0
**  ret
*/
float test_builtin10(float x) {
  return __builtin_riscv_mips_ffsqrt(x);
}

/*
**test_builtin14:
**	mips.flti.s fa5,8
**	fmul.s	fa5,fa0,fa5
**	mips.ffexp2.s fa0,fa5
**	ret
*/
float test_builtin14(float x) {
  return __builtin_riscv_mips_exp10f(x);
}

/*
**test_builtin15:
**  fmul.s	fa5,fa0,fa0
**  mips.ffrecip.s fa4,fa0
**  fli.s	fa3,1.0
**  fsub.s	fa5,fa3,fa5
**	mips.ffsqrt.s fa3,fa5
**  mips.ffrsqrt.s fa5,fa5
**	fmul.s	fa3,fa4,fa3
**  fmul.s	fa5,fa0,fa5
**	mips.fatanhz.s fa3,fa3
**	fli.s	fa4,0.25
**	fsgnj.s	fa4,fa4,fa0
**	mips.fatanhz.s fa5,fa5
**	fsub.s	fa0,fa4,fa3
**	fmax.s	fa0,fa5,fa0
**  ret
*/
float test_builtin15(float x) {
  return __builtin_riscv_mips_asinf_hz(x);
}

/*
**test_builtin17:
**	fmul.s	fa5,fa0,fa0
**	mips.ffrecip.s fa4,fa0
**	fli.s	fa3,1.0
**	fsub.s	fa5,fa3,fa5
**	mips.ffsqrt.s fa3,fa5
**	mips.ffrsqrt.s fa5,fa5
**	fmul.s	fa3,fa4,fa3
**	fmul.s	fa5,fa0,fa5
**	mips.fatanhz.s fa3,fa3
**	fli.s	fa4,0.25
**	fsgnj.s	fa4,fa4,fa0
**	mips.fatanhz.s fa5,fa5
**	fsub.s	fa4,fa4,fa3
**	fmax.s	fa4,fa5,fa4
**	fli.s	fa0,0.5
**	fsub.s	fa0,fa0,fa4
**	ret
*/
float test_builtin17(float x) {
  return __builtin_riscv_mips_acosf_hz(x);
}

/*
**test_builtin20:
**	fdiv.s	fa0,fa0,fa1
**	mips.fatanhz.s fa0,fa0
**	ret
*/
float test_builtin20(float x, float y) {
  return __builtin_riscv_mips_atan2f_hz(x, y);
}

/*
**test_builtin22:
**	fli.s	fa5,0.25
**	fsub.s	fa5,fa5,fa0
**	mips.ftanhz.s fa0,fa0
**	mips.ftanhz.s fa5,fa5
**	mips.ffrecip.s fa5,fa5
**	fmax.s	fa0,fa0,fa5
**	ret
*/
float test_builtin22(float x) {
  return __builtin_riscv_mips_tanf_hz_fc(x);
}

/*
**test_builtin23:
**  mips.ffrecip.s fa5,fa0
**  mips.fatanhz.s fa5,fa5
**  fli.s	fa4,0.25
**  fsub.s	fa5,fa4,fa5
**  mips.fatanhz.s fa0,fa0
**  fmax.s	fa0,fa0,fa5
**  ret
*/
float test_builtin23(float x) {
  return __builtin_riscv_mips_atanf_hz_fc(x);
}
