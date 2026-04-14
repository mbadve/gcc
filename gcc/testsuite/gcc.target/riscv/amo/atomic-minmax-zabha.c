/* { dg-do run } */
/* { dg-options "-march=rv32imafdb_zabha --save-temps -latomic" } */
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
**      amomin\.b\ta[0-7],a[0-7],0\(a[0-7]\)
**      ...
*/

/*
** test_smax_s8:
**      ...
**      amomax\.b\.rl\ta[0-7],a[0-7],0\(a[0-7]\)
**      ...
*/

/*
** test_smin_s16:
**      ...
**      amomin\.h\.aq\ta[0-7],a[0-7],0\(a[0-7]\)
**      ...
*/

/*
** test_smax_s16:
**      ...
**      amomax\.h\.aqrl\ta[0-7],a[0-7],0\(a[0-7]\)
**      ...
*/

/*
** test_smin_s32:
**      ...
**      amomin\.w\ta[0-7],a[0-7],0\(a[0-7]\)
**      ...
*/

/*
** test_smax_s32:
**      ...
**      amomax\.w\.aqrl\ta[0-7],a[0-7],0\(a[0-7]\)
**      ...
*/

/*
** test_smin_s64:
**      ...
**      call\t__atomic_compare_exchange_8
**      ...
*/

/*
** test_smax_s64:
**      ...
**      call\t__atomic_compare_exchange_8
**      ...
*/

/*
** test_umin_u8:
**      ...
**      amominu\.b\ta[0-7],a[0-7],0\(a[0-7]\)
**      ...
*/

/*
** test_umax_u8:
**      ...
**      amomaxu\.b\.aq\ta[0-7],a[0-7],0\(a[0-7]\)
**      ...
*/

/*
** test_umin_u16:
**      ...
**      amominu\.h\.aq\ta[0-7],a[0-7],0\(a[0-7]\)
**      ...
*/

/*
** test_umax_u16:
**      ...
**      amomaxu\.h\.rl\ta[0-7],a[0-7],0\(a[0-7]\)
**      ...
*/

/*
** test_umin_u32:
**      ...
**      amominu\.w\.aqrl\ta[0-7],a[0-7],0\(a[0-7]\)
**      ...
*/

/*
** test_umax_u32:
**      ...
**      amomaxu\.w\ta[0-7],a[0-7],0\(a[0-7]\)
**      ...
*/

/*
** test_umin_u64:
**      ...
**      call\t__atomic_compare_exchange_8
**      ...
*/

/*
** test_umax_u64:
**      ...
**      call\t__atomic_compare_exchange_8
**      ...
*/
