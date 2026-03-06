/* { dg-do compile } */
/* { dg-options "-march=rv32imafd_zcmp" } */
/* { dg-skip-if "" { *-*-* } { "-O0" "-O1" "-O2" "-O3" "-Og" "-Oz" } } */

long long int test (int i0, int i1, int j0)
{
  long long int result;

  result = ((long long int) (i0) << j0) | ((long long int) i1 << (j0 - 52));

  return result;
}
/* { dg-final { scan-assembler "cm.mvsa01" } } */
