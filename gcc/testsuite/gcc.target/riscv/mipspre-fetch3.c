/* { dg-do compile } */
/* { dg-options "-march=rv32gc_zicbop -mtune=mips-m8500" { target { rv32 } } } */
/* { dg-skip-if "" { riscv64-*-* } { } } */

void test1(char *a)
{
  __builtin_prefetch (&a[0], 0, 0);
}

void test2(char *a)
{
  __builtin_prefetch (&a[1], 1, 0);
}

void test3(char *a)
{
  __builtin_prefetch (&a[511], 0, 1);
}

void test4(char *a)
{
  __builtin_prefetch (&a[512], 1, 1);
}

/* { dg-final { scan-assembler-times "prefetch.r" 2 } } */
/* { dg-final { scan-assembler-times "prefetch.w" 2 } } */
