/* { dg-do compile } */
/* { dg-options "-O2 -fdump-tree-cas-to-minmax" } */

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

/* { dg-final { scan-tree-dump "ATOMIC_FETCH_MINMAX" "cas-to-minmax" } } */
/* { dg-final { scan-tree-dump-not "ATOMIC_COMPARE_EXCHANGE" "cas-to-minmax" } }
 */