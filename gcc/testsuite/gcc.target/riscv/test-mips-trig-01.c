/* { dg-do compile } */
/* { dg-options "-march=rv32imafd_xmipstrig_zfa -mtune=mips-m8500 -mabi=ilp32d -ffast-math" } */
/* { dg-skip-if "" { *-*-* } { "-O0" "-O2" "-Os" "-Oz" "-g" "-flto" } } */
/* { dg-final { check-function-bodies "**" "" } } */
/* MIPS riscv cpu m8500 - test trig support */

/*
**test_sinf:
**	mips.flti.s fa5,5
**	fmul.s	fa5,fa0,fa5
**	mips.fsinhz.s fa0,fa5
**	ret
*/
extern float sinf(float x);
float test_sinf(float x) {
  return sinf(x);
}

/*
**test_cosf:
**	mips.flti.s fa5,5
**	fmul.s	fa5,fa0,fa5
**	mips.fcoshz.s fa0,fa5
**	ret
*/
extern float cosf(float x);
float test_cosf(float x) {
  return cosf(x);
}

/*
**test_tanf:
**	mips.flti.s fa5,5
**	fmul.s	fa5,fa0,fa5
**	mips.ftanhz.s fa0,fa5
**	ret
*/
extern float tanf(float x);
float test_tanf(float x) {
  return tanf(x);
}

/*
**test_atanf:
**	mips.fatanhz.s fa0,fa0
**	mips.flti.s fa5,3
**	fmul.s	fa0,fa0,fa5
**	ret
*/
extern float atanf(float x);
float test_atanf(float x) {
  return atanf(x);
}

/*
**test_tanh:
**	mips.fftanh.s fa0,fa0
**	ret
*/
extern float tanhf(float x);
float test_tanh(float x) {
  return tanhf(x);
}

/*
**test_log2f:
**	mips.fflog2.s fa0,fa0
**	ret
*/
extern float log2f(float x);
float test_log2f(float x) {
  return log2f(x);
}

/*
**test_logf:
** mips.fflog2.s fa0,fa0
** mips.flti.s fa5,6
** fmul.s	fa0,fa0,fa5
** ret
*/
extern float logf(float x);
float test_logf(float x) {
  return logf(x);
}

/*
**test_log10:
** mips.fflog2.s fa0,fa0
** mips.flti.s fa5,9
** fmul.s	fa0,fa0,fa5
** ret
*/
extern float log10f(float x);
float test_log10(float x) {
  return log10f(x);
}

/*
**test_expf:
**	mips.flti.s fa5,7
**	fmul.s	fa5,fa0,fa5
**	mips.ffexp2.s fa0,fa5
**	ret
*/
extern float expf(float x);
float test_expf(float x) {
  return expf(x);
}

/*
**test_exp10f:
**	mips.flti.s fa5,8
**	fmul.s	fa5,fa0,fa5
**	mips.ffexp2.s fa0,fa5
**	ret
*/
extern float exp10f(float x);
float test_exp10f(float x) {
  return exp10f(x);
}

/*
**test_asinf:
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
**	mips.flti.s fa0,3
**	fmul.s	fa0,fa4,fa0
**	ret
*/
extern float asinf(float x);
float test_asinf(float x) {
  return asinf(x);
}

/*
**test_acosf:
**	fmul.s	fa5,fa0,fa0
**	mips.ffrecip.s fa4,fa0
**	fli.s	fa3,1.0
**	fsub.s	fa5,fa3,fa5
**	mips.ffsqrt.s fa3,fa5
**	mips.ffrsqrt.s fa5,fa5
**	fmul.s	fa3,fa4,fa3
**	fmul.s	fa5,fa0,fa5
**	mips.fatanhz.s fa3,fa3
**	fli.s	fa2,0.25
**	fsgnj.s	fa4,fa2,fa0
**	mips.fatanhz.s fa5,fa5
**	fsub.s	fa4,fa4,fa3
**	fmax.s	fa4,fa5,fa4
**	fsub.s	fa4,fa2,fa4
**	mips.flti.s fa0,3
**	fmul.s	fa0,fa4,fa0
**	ret
*/
extern float acosf(float x);
float test_acosf(float x) {
  return acosf(x);
}

/*
**test_atan2f:
**	fdiv.s	fa0,fa0,fa1
**	mips.fatanhz.s fa0,fa0
**	mips.flti.s fa5,3
**	fmul.s	fa0,fa0,fa5
**	ret
*/
extern float atan2f(float x, float y);
float test_atan2f(float x, float y) {
  return atan2f(x, y);
}

/*
**test_hypotf:
**  fmul.s	fa0,fa0,fa0
**	fmul.s	fa1,fa1,fa1
**	fadd.s	fa1,fa1,fa0
**	mips.ffsqrt.s fa0,fa1
**	ret
*/
extern float hypotf(float x, float y);
float test_hypotf(float x, float y) {
  return hypotf(x, y);
}
