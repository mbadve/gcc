/* { dg-do run } */
/* { dg-options "-O2" } */

extern void abort (void);

unsigned int
atomic_fetch_max_unsigned (unsigned int *ptr, unsigned int val)
{
  unsigned int expected = __atomic_load_n (ptr, __ATOMIC_RELAXED);
  while (!__atomic_compare_exchange_n (ptr, &expected,
				       expected > val ? expected : val, 0,
				       __ATOMIC_SEQ_CST, __ATOMIC_RELAXED))
    ;
  return expected;
}

unsigned int
atomic_fetch_min_unsigned (unsigned int *ptr, unsigned int val)
{
  unsigned int expected = __atomic_load_n (ptr, __ATOMIC_RELAXED);
  while (!__atomic_compare_exchange_n (ptr, &expected,
				       expected < val ? expected : val, 0,
				       __ATOMIC_SEQ_CST, __ATOMIC_RELAXED))
    ;
  return expected;
}

int
main ()
{
  unsigned int x;
  unsigned int old;

  x = 100u;
  old = atomic_fetch_max_unsigned (&x, 200u);
  if (old != 100u || x != 200u)
    abort ();

  x = 100u;
  old = atomic_fetch_min_unsigned (&x, 50u);
  if (old != 100u || x != 50u)
    abort ();

  x = 0xFFFFFFFFu;
  old = atomic_fetch_min_unsigned (&x, 0u);
  if (old != 0xFFFFFFFFu || x != 0u)
    abort ();

  x = 0u;
  old = atomic_fetch_max_unsigned (&x, 0xFFFFFFFFu);
  if (old != 0u || x != 0xFFFFFFFFu)
    abort ();

  return 0;
}
