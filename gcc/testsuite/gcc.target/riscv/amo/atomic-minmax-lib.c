/* { dg-do run } */
/* { dg-options "-march=rv32imfdb --save-temps -latomic" } */
/* { dg-skip-if "" { *-*-* } { "-flto"} } */
/* { dg-final { check-function-bodies "**" "" } } */

#include "atomic-minmax.x"

int main ()
{
  run_tests();
  return 0;
}

/*
** test_smin_s8:
**      ...
**      call\t__atomic_compare_exchange_1
**      ...
*/

/*
** test_smax_s8:
**      ...
**      call\t__atomic_compare_exchange_1
**      ...
*/

/*
** test_smin_s16:
**      ...
**      call\t__atomic_compare_exchange_2
**      ...
*/

/*
** test_smax_s16:
**      ...
**      call\t__atomic_compare_exchange_2
**      ...
*/

/*
** test_smin_s32:
**      ...
**      call\t__atomic_compare_exchange_4
**      ...
*/

/*
** test_smax_s32:
**      ...
**      call\t__atomic_compare_exchange_4
**      ...
*/

/*
** test_umin_u8:
**      ...
**      call\t__atomic_compare_exchange_1
**      ...
*/

/*
** test_umax_u8:
**      ...
**      call\t__atomic_compare_exchange_1
**      ...
*/

/*
** test_umin_u16:
**      ...
**      call\t__atomic_compare_exchange_2
**      ...
*/

/*
** test_umax_u16:
**      ...
**      call\t__atomic_compare_exchange_2
**      ...
*/

/*
** test_umin_u32:
**      ...
**      call\t__atomic_compare_exchange_4
**      ...
*/

/*
** test_umax_u32:
**      ...
**      call\t__atomic_compare_exchange_4
**      ...
*/
