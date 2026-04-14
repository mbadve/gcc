/* { dg-do run } */
/* { dg-options "-march=rv32imafdb --save-temps -latomic" } */
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
**      lr\.w\ta[0-7], 0\(a[0-7]\)
**      and\t[at][0-9]+, a[0-7], a[0-7]
**      min\ta[0-7], [at][0-9]+, a[0-7]
**      ...
*/

/*
** test_smax_s8:
**      ...
**      lr\.w\ta[0-7], 0\(a[0-7]\)
**      and\t[at][0-9]+, a[0-7], a[0-7]
**      max\ta[0-7], [at][0-9]+, a[0-7]
**      ...
*/

/*
** test_smin_s16:
**      ...
**      lr\.w\.aq\ta[0-7], 0\(a[0-7]\)
**      and\t[at][0-9]+, a[0-7], a[0-7]
**      min\ta[0-7], [at][0-9]+, a[0-7]
**      ...
*/

/*
** test_smax_s16:
**      ...
**      lr\.w\.aq\ta[0-7], 0\(a[0-7]\)
**      and\t[at][0-9]+, a[0-7], a[0-7]
**      max\ta[0-7], [at][0-9]+, a[0-7]
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
**      lr\.w\ta[0-7], 0\(a[0-7]\)
**      and\t[at][0-9]+, a[0-7], a[0-7]
**      minu\ta[0-7], [at][0-9]+, a[0-7]
**      ...
*/

/*
** test_umax_u8:
**      ...
**      lr\.w\.aq\ta[0-7], 0\(a[0-7]\)
**      and\t[at][0-9]+, a[0-7], a[0-7]
**      maxu\ta[0-7], [at][0-9]+, a[0-7]
**      ...
*/

/*
** test_umin_u16:
**      ...
**      lr\.w\.aq\ta[0-7], 0\(a[0-7]\)
**      and\t[at][0-9]+, a[0-7], a[0-7]
**      minu\ta[0-7], [at][0-9]+, a[0-7]
**      ...
*/

/*
** test_umax_u16:
**      ...
**      lr\.w\ta[0-7], 0\(a[0-7]\)
**      and\t[at][0-9]+, a[0-7], a[0-7]
**      maxu\ta[0-7], [at][0-9]+, a[0-7]
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
