/* { dg-do run } */
/* { dg-options "-O2" } */

extern void abort (void);

int
atomic_fetch_min (int *ptr, int val)
{
  int expected = __atomic_load_n (ptr, __ATOMIC_RELAXED);
  while (!__atomic_compare_exchange_n (ptr, &expected,
				       expected < val ? expected : val, 0,
				       __ATOMIC_SEQ_CST, __ATOMIC_RELAXED))
    ;
  return expected;
}

int
atomic_fetch_max (int *ptr, int val)
{
  int expected = __atomic_load_n (ptr, __ATOMIC_RELAXED);
  while (!__atomic_compare_exchange_n (ptr, &expected,
				       expected > val ? expected : val, 0,
				       __ATOMIC_SEQ_CST, __ATOMIC_RELAXED))
    ;
  return expected;
}

int
main ()
{
  int x;
  int old;

  x = 10;
  old = atomic_fetch_min (&x, 5);
  if (old != 10 || x != 5)
    abort ();

  x = 10;
  old = atomic_fetch_min (&x, 15);
  if (old != 10 || x != 10)
    abort ();

  x = 10;
  old = atomic_fetch_max (&x, 20);
  if (old != 10 || x != 20)
    abort ();

  x = 10;
  old = atomic_fetch_max (&x, 5);
  if (old != 10 || x != 10)
    abort ();

  x = -5;
  old = atomic_fetch_min (&x, -10);
  if (old != -5 || x != -10)
    abort ();

  x = -10;
  old = atomic_fetch_max (&x, -5);
  if (old != -10 || x != -5)
    abort ();

  x = 42;
  old = atomic_fetch_min (&x, 42);
  if (old != 42 || x != 42)
    abort ();

  x = 42;
  old = atomic_fetch_max (&x, 42);
  if (old != 42 || x != 42)
    abort ();

  return 0;
}
