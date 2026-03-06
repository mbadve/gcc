/* { dg-do compile } */
/* { dg-options "-mremove-back-to-back-branches" } */
/* { dg-skip-if "" { *-*-* } { "-O0" "-O1" "-O2" "-O3" } } */

void 
foo (int a, int b)
{
  if (a > 0)
  {
    if (b <= 0)
    {
      __asm__ __volatile__("nop");
    }
  }
}
/* { dg-final { scan-assembler "# branch to branch hazard\n\t.align\t3" } } */
