/* { dg-do compile } */
/* { dg-options "-O2 -fdump-tree-cas-to-minmax" } */

void
atomic_min_no_return (int *ptr, int val)
{
  int expected = __atomic_load_n (ptr, __ATOMIC_RELAXED);
  while (!__atomic_compare_exchange_n (ptr, &expected,
				       expected < val ? expected : val, 0,
				       __ATOMIC_SEQ_CST, __ATOMIC_RELAXED))
    ;
}

/* { dg-final { scan-tree-dump "ATOMIC_FETCH_MINMAX" "cas-to-minmax" } } */