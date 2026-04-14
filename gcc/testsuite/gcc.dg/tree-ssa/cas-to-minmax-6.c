/* { dg-do compile } */
/* { dg-options "-O2 -fdump-tree-cas-to-minmax" } */

int
atomic_fetch_max_mem_order (int *ptr, int val)
{
  int expected = __atomic_load_n (ptr, __ATOMIC_ACQUIRE);
  while (!__atomic_compare_exchange_n (ptr, &expected,
				       expected > val ? expected : val, 0,
				       __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
    ;
  return expected;
}

/* { dg-final { scan-tree-dump "ATOMIC_FETCH_MINMAX" "cas-to-minmax" } } */
/* { dg-final { scan-tree-dump "ATOMIC_FETCH_MINMAX \\(ptr_\[0-9\]+\\(D\\),
 * val_\[0-9\]+\\(D\\), 4," "cas-to-minmax" } } */
/* { dg-final { scan-tree-dump-not "ATOMIC_COMPARE_EXCHANGE" "cas-to-minmax" } }
 */