/* { dg-do compile } */
/* { dg-options "-march=rv32imafd -mtune=mips-m8500 -mabi=ilp32d" } */
/* { dg-skip-if "" { *-*-* } { "-O0" "-g" } } */
/* MIPS riscv cpu m8500 trig builtins - test for error */
float test_builtin1(float x) {
  return __builtin_riscv_mips_fsinhz(x); /* { dg-error "implicit declaration of function" } */
}

float test_builtin2(float x) {
  return __builtin_riscv_mips_fcoshz(x); /* { dg-error "implicit declaration of function" } */
}

float test_builtin3(float x) {
  return __builtin_riscv_mips_ftanhz(x); /* { dg-error "implicit declaration of function" } */
}

float test_builtin4(float x) {
  return __builtin_riscv_mips_fversinhz(x); /* { dg-error "implicit declaration of function" } */
}

float test_builtin5(float x) {
  return __builtin_riscv_mips_ffrecip(x); /* { dg-error "implicit declaration of function" } */
}

float test_builtin6(float x) {
  return __builtin_riscv_mips_ffrsqrt(x); /* { dg-error "implicit declaration of function" } */
}

float test_builtin7(float x) {
  return __builtin_riscv_mips_fatanhz(x); /* { dg-error "implicit declaration of function" } */
}

float test_builtin8(float x) {
  return __builtin_riscv_mips_ffexp2(x); /* { dg-error "implicit declaration of function" } */
}

float test_builtin9(float x) {
  return __builtin_riscv_mips_ffsqrt(x); /* { dg-error "implicit declaration of function" } */
}

float test_builtin10(float x) {
  return __builtin_riscv_mips_exp10f(x); /* { dg-error "implicit declaration of function" } */
}

float test_builtin11(float x) {
  return __builtin_riscv_mips_asinf_hz(x); /* { dg-error "implicit declaration of function" } */
}

float test_builtin12(float x) {
  return __builtin_riscv_mips_acosf_hz(x); /* { dg-error "implicit declaration of function" } */
}

float test_builtin13(float x, float y) {
  return __builtin_riscv_mips_atan2f_hz(x, y); /* { dg-error "implicit declaration of function" } */
}

float test_builtin14(float x) {
  return __builtin_riscv_mips_tanf_hz_fc(x); /* { dg-error "implicit declaration of function" } */
}

float test_builtin15(float x) {
  return __builtin_riscv_mips_atanf_hz_fc(x); /* { dg-error "implicit declaration of function" } */
}
