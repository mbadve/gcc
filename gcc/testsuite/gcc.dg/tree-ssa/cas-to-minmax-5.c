/* { dg-do compile } */
/* { dg-options "-O2 -fdump-tree-cas-to-minmax" } */

extern void log_attempt (int);

int
atomic_max_side_effect (int *ptr, int val)
{
  int expected = __atomic_load_n (ptr, __ATOMIC_RELAXED);
  while (!__atomic_compare_exchange_n (ptr, &expected,
				       expected > val ? expected : val, 0,
				       __ATOMIC_SEQ_CST, __ATOMIC_RELAXED))
    {
      log_attempt (expected); /* Side effect, do not transform.  */
    }
  return expected;
}

/* { dg-final { scan-tree-dump-not "ATOMIC_FETCH_MINMAX" "cas-to-minmax" } } */