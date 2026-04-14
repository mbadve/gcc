/* Test __atomic routines for existence and proper execution on 2 byte 
   values with each valid memory model.  */
/* { dg-do run } */
/* { dg-require-effective-target sync_char_short } */


/* Test the execution of the __atomic_*OP builtin routines for a short.  */

#include "limits.h"

extern void abort(void);

short v, count, res;
const short init = ~0;

/* The fetch_op routines return the original value before the operation.  */

void
test_fetch_add ()
{
  v = 0;
  count = 1;

  if (__atomic_fetch_add (&v, count, __ATOMIC_RELAXED) != 0)
    abort ();

  if (__atomic_fetch_add (&v, 1, __ATOMIC_CONSUME) != 1) 
    abort ();

  if (__atomic_fetch_add (&v, count, __ATOMIC_ACQUIRE) != 2)
    abort ();

  if (__atomic_fetch_add (&v, 1, __ATOMIC_RELEASE) != 3) 
    abort ();

  if (__atomic_fetch_add (&v, count, __ATOMIC_ACQ_REL) != 4) 
    abort ();

  if (__atomic_fetch_add (&v, 1, __ATOMIC_SEQ_CST) != 5) 
    abort ();
}


void
test_fetch_sub()
{
  v = res = 20;
  count = 0;

  if (__atomic_fetch_sub (&v, count + 1, __ATOMIC_RELAXED) !=  res--) 
    abort ();

  if (__atomic_fetch_sub (&v, 1, __ATOMIC_CONSUME) !=  res--) 
    abort ();

  if (__atomic_fetch_sub (&v, count + 1, __ATOMIC_ACQUIRE) !=  res--) 
    abort ();

  if (__atomic_fetch_sub (&v, 1, __ATOMIC_RELEASE) !=  res--) 
    abort ();

  if (__atomic_fetch_sub (&v, count + 1, __ATOMIC_ACQ_REL) !=  res--) 
    abort ();

  if (__atomic_fetch_sub (&v, 1, __ATOMIC_SEQ_CST) !=  res--) 
    abort ();
}

void
test_fetch_and ()
{
  v = init;

  if (__atomic_fetch_and (&v, 0, __ATOMIC_RELAXED) !=  init) 
    abort ();

  if (__atomic_fetch_and (&v, init, __ATOMIC_CONSUME) !=  0) 
    abort ();

  if (__atomic_fetch_and (&v, 0, __ATOMIC_ACQUIRE) !=  0)
    abort ();

  v = ~v;
  if (__atomic_fetch_and (&v, init, __ATOMIC_RELEASE) !=  init)
    abort ();

  if (__atomic_fetch_and (&v, 0, __ATOMIC_ACQ_REL) !=  init) 
    abort ();

  if (__atomic_fetch_and (&v, 0, __ATOMIC_SEQ_CST) !=  0) 
    abort ();
}

void
test_fetch_nand ()
{
  v = init;

  if (__atomic_fetch_nand (&v, 0, __ATOMIC_RELAXED) !=  init) 
    abort ();

  if (__atomic_fetch_nand (&v, init, __ATOMIC_CONSUME) !=  init) 
    abort ();

  if (__atomic_fetch_nand (&v, 0, __ATOMIC_ACQUIRE) !=  0 ) 
    abort ();

  if (__atomic_fetch_nand (&v, init, __ATOMIC_RELEASE) !=  init)
    abort ();

  if (__atomic_fetch_nand (&v, init, __ATOMIC_ACQ_REL) !=  0) 
    abort ();

  if (__atomic_fetch_nand (&v, 0, __ATOMIC_SEQ_CST) !=  init) 
    abort ();
}

void
test_fetch_xor ()
{
  v = init;
  count = 0;

  if (__atomic_fetch_xor (&v, count, __ATOMIC_RELAXED) !=  init) 
    abort ();

  if (__atomic_fetch_xor (&v, ~count, __ATOMIC_CONSUME) !=  init) 
    abort ();

  if (__atomic_fetch_xor (&v, 0, __ATOMIC_ACQUIRE) !=  0) 
    abort ();

  if (__atomic_fetch_xor (&v, ~count, __ATOMIC_RELEASE) !=  0) 
    abort ();

  if (__atomic_fetch_xor (&v, 0, __ATOMIC_ACQ_REL) !=  init) 
    abort ();

  if (__atomic_fetch_xor (&v, ~count, __ATOMIC_SEQ_CST) !=  init) 
    abort ();
}

void
test_fetch_or ()
{
  v = 0;
  count = 1;

  if (__atomic_fetch_or (&v, count, __ATOMIC_RELAXED) !=  0) 
    abort ();

  count *= 2;
  if (__atomic_fetch_or (&v, 2, __ATOMIC_CONSUME) !=  1) 
    abort ();

  count *= 2;
  if (__atomic_fetch_or (&v, count, __ATOMIC_ACQUIRE) !=  3) 
    abort ();

  count *= 2;
  if (__atomic_fetch_or (&v, 8, __ATOMIC_RELEASE) !=  7) 
    abort ();

  count *= 2;
  if (__atomic_fetch_or (&v, count, __ATOMIC_ACQ_REL) !=  15) 
    abort ();

  count *= 2;
  if (__atomic_fetch_or (&v, count, __ATOMIC_SEQ_CST) !=  31) 
    abort ();
}

void
test_fetch_smin ()
{
  short sv = 10;

  if (__atomic_fetch_min (&sv, 5, __ATOMIC_RELAXED) != 10)
    abort ();

  if (__atomic_fetch_min (&sv, 5, __ATOMIC_CONSUME) != 5)
    abort ();

  if (__atomic_fetch_min (&sv, -10, __ATOMIC_ACQUIRE) != 5)
    abort ();

  if (__atomic_fetch_min (&sv, -20, __ATOMIC_RELEASE) != -10)
    abort ();

  if (__atomic_fetch_min (&sv, SHRT_MIN, __ATOMIC_ACQ_REL) != -20)
    abort ();

  if (__atomic_fetch_min (&sv, SHRT_MAX, __ATOMIC_SEQ_CST) != SHRT_MIN)
    abort ();

  if (sv != SHRT_MIN)
    abort ();
}

void
test_fetch_umin ()
{
  unsigned short uv = 100;

  if (__atomic_fetch_min (&uv, 50, __ATOMIC_RELAXED) != 100)
    abort ();

  if (__atomic_fetch_min (&uv, 50, __ATOMIC_CONSUME) != 50)
    abort ();

  if (__atomic_fetch_min (&uv, 75, __ATOMIC_ACQUIRE) != 50)
    abort ();

  if (__atomic_fetch_min (&uv, 10, __ATOMIC_RELEASE) != 50)
    abort ();

  if (__atomic_fetch_min (&uv, 0, __ATOMIC_ACQ_REL) != 10)
    abort ();

  if (__atomic_fetch_min (&uv, USHRT_MAX, __ATOMIC_SEQ_CST) != 0)
    abort ();

  if (uv != 0)
    abort ();
}

void
test_fetch_smax ()
{
  short sv = -10;

  if (__atomic_fetch_max (&sv, -5, __ATOMIC_RELAXED) != -10)
    abort ();

  if (__atomic_fetch_max (&sv, -5, __ATOMIC_CONSUME) != -5)
    abort ();

  if (__atomic_fetch_max (&sv, -20, __ATOMIC_ACQUIRE) != -5)
    abort ();

  if (__atomic_fetch_max (&sv, 10, __ATOMIC_RELEASE) != -5)
    abort ();

  if (__atomic_fetch_max (&sv, 50, __ATOMIC_ACQ_REL) != 10)
    abort ();

  if (__atomic_fetch_max (&sv, SHRT_MAX, __ATOMIC_SEQ_CST) != 50)
    abort ();

  if (sv != SHRT_MAX)
    abort ();
}

void
test_fetch_umax ()
{
  unsigned short uv = 10;

  if (__atomic_fetch_max (&uv, 50, __ATOMIC_RELAXED) != 10)
    abort ();

  if (__atomic_fetch_max (&uv, 50, __ATOMIC_CONSUME) != 50)
    abort ();

  if (__atomic_fetch_max (&uv, 30, __ATOMIC_ACQUIRE) != 50)
    abort ();

  if (__atomic_fetch_max (&uv, 100, __ATOMIC_RELEASE) != 50)
    abort ();

  if (__atomic_fetch_max (&uv, 200, __ATOMIC_ACQ_REL) != 100)
    abort ();

  if (__atomic_fetch_max (&uv, USHRT_MAX, __ATOMIC_SEQ_CST) != 200)
    abort ();

  if (uv != USHRT_MAX)
    abort ();
}

/* The OP_fetch routines return the new value after the operation.  */

void
test_add_fetch ()
{
  v = 0;
  count = 1;

  if (__atomic_add_fetch (&v, count, __ATOMIC_RELAXED) != 1)
    abort ();

  if (__atomic_add_fetch (&v, 1, __ATOMIC_CONSUME) != 2) 
    abort ();

  if (__atomic_add_fetch (&v, count, __ATOMIC_ACQUIRE) != 3)
    abort ();

  if (__atomic_add_fetch (&v, 1, __ATOMIC_RELEASE) != 4) 
    abort ();

  if (__atomic_add_fetch (&v, count, __ATOMIC_ACQ_REL) != 5) 
    abort ();

  if (__atomic_add_fetch (&v, count, __ATOMIC_SEQ_CST) != 6) 
    abort ();
}


void
test_sub_fetch ()
{
  v = res = 20;
  count = 0;

  if (__atomic_sub_fetch (&v, count + 1, __ATOMIC_RELAXED) !=  --res) 
    abort ();

  if (__atomic_sub_fetch (&v, 1, __ATOMIC_CONSUME) !=  --res) 
    abort ();                                                  
                                                               
  if (__atomic_sub_fetch (&v, count + 1, __ATOMIC_ACQUIRE) !=  --res) 
    abort ();                                                  
                                                               
  if (__atomic_sub_fetch (&v, 1, __ATOMIC_RELEASE) !=  --res) 
    abort ();                                                  
                                                               
  if (__atomic_sub_fetch (&v, count + 1, __ATOMIC_ACQ_REL) !=  --res) 
    abort ();                                                  
                                                               
  if (__atomic_sub_fetch (&v, count + 1, __ATOMIC_SEQ_CST) !=  --res) 
    abort ();
}

void
test_and_fetch ()
{
  v = init;

  if (__atomic_and_fetch (&v, 0, __ATOMIC_RELAXED) !=  0) 
    abort ();

  v = init;
  if (__atomic_and_fetch (&v, init, __ATOMIC_CONSUME) !=  init) 
    abort ();

  if (__atomic_and_fetch (&v, 0, __ATOMIC_ACQUIRE) !=  0) 
    abort ();

  v = ~v;
  if (__atomic_and_fetch (&v, init, __ATOMIC_RELEASE) !=  init)
    abort ();

  if (__atomic_and_fetch (&v, 0, __ATOMIC_ACQ_REL) !=  0) 
    abort ();

  v = ~v;
  if (__atomic_and_fetch (&v, 0, __ATOMIC_SEQ_CST) !=  0) 
    abort ();
}

void
test_nand_fetch ()
{
  v = init;

  if (__atomic_nand_fetch (&v, 0, __ATOMIC_RELAXED) !=  init) 
    abort ();              
                           
  if (__atomic_nand_fetch (&v, init, __ATOMIC_CONSUME) !=  0) 
    abort ();              
                           
  if (__atomic_nand_fetch (&v, 0, __ATOMIC_ACQUIRE) !=  init) 
    abort ();              
                           
  if (__atomic_nand_fetch (&v, init, __ATOMIC_RELEASE) !=  0)
    abort ();              
                           
  if (__atomic_nand_fetch (&v, init, __ATOMIC_ACQ_REL) !=  init) 
    abort ();              
                           
  if (__atomic_nand_fetch (&v, 0, __ATOMIC_SEQ_CST) !=  init) 
    abort ();
}



void
test_xor_fetch ()
{
  v = init;
  count = 0;

  if (__atomic_xor_fetch (&v, count, __ATOMIC_RELAXED) !=  init) 
    abort ();

  if (__atomic_xor_fetch (&v, ~count, __ATOMIC_CONSUME) !=  0) 
    abort ();

  if (__atomic_xor_fetch (&v, 0, __ATOMIC_ACQUIRE) !=  0) 
    abort ();

  if (__atomic_xor_fetch (&v, ~count, __ATOMIC_RELEASE) !=  init) 
    abort ();

  if (__atomic_xor_fetch (&v, 0, __ATOMIC_ACQ_REL) !=  init) 
    abort ();

  if (__atomic_xor_fetch (&v, ~count, __ATOMIC_SEQ_CST) !=  0) 
    abort ();
}

void
test_or_fetch ()
{
  v = 0;
  count = 1;

  if (__atomic_or_fetch (&v, count, __ATOMIC_RELAXED) !=  1) 
    abort ();

  count *= 2;
  if (__atomic_or_fetch (&v, 2, __ATOMIC_CONSUME) !=  3) 
    abort ();

  count *= 2;
  if (__atomic_or_fetch (&v, count, __ATOMIC_ACQUIRE) !=  7) 
    abort ();

  count *= 2;
  if (__atomic_or_fetch (&v, 8, __ATOMIC_RELEASE) !=  15) 
    abort ();

  count *= 2;
  if (__atomic_or_fetch (&v, count, __ATOMIC_ACQ_REL) !=  31) 
    abort ();

  count *= 2;
  if (__atomic_or_fetch (&v, count, __ATOMIC_SEQ_CST) !=  63) 
    abort ();
}

void
test_smin_fetch ()
{
  short sv = 10;

  if (__atomic_min_fetch (&sv, 5, __ATOMIC_RELAXED) != 5)
    abort ();

  if (__atomic_min_fetch (&sv, 5, __ATOMIC_CONSUME) != 5)
    abort ();

  if (__atomic_min_fetch (&sv, -10, __ATOMIC_ACQUIRE) != -10)
    abort ();

  if (__atomic_min_fetch (&sv, -20, __ATOMIC_RELEASE) != -20)
    abort ();

  if (__atomic_min_fetch (&sv, SHRT_MIN, __ATOMIC_ACQ_REL) != SHRT_MIN)
    abort ();

  if (__atomic_min_fetch (&sv, SHRT_MAX, __ATOMIC_SEQ_CST) != SHRT_MIN)
    abort ();

  if (sv != SHRT_MIN)
    abort ();
}

void
test_umin_fetch ()
{
  unsigned short uv = 100;

  if (__atomic_min_fetch (&uv, 50, __ATOMIC_RELAXED) != 50)
    abort ();

  if (__atomic_min_fetch (&uv, 50, __ATOMIC_CONSUME) != 50)
    abort ();

  if (__atomic_min_fetch (&uv, 75, __ATOMIC_ACQUIRE) != 50)
    abort ();

  if (__atomic_min_fetch (&uv, 10, __ATOMIC_RELEASE) != 10)
    abort ();

  if (__atomic_min_fetch (&uv, 0, __ATOMIC_ACQ_REL) != 0)
    abort ();

  if (__atomic_min_fetch (&uv, USHRT_MAX, __ATOMIC_SEQ_CST) != 0)
    abort ();

  if (uv != 0)
    abort ();
}

void
test_smax_fetch ()
{
  short sv = -10;

  if (__atomic_max_fetch (&sv, -5, __ATOMIC_RELAXED) != -5)
    abort ();

  if (__atomic_max_fetch (&sv, -5, __ATOMIC_CONSUME) != -5)
    abort ();

  if (__atomic_max_fetch (&sv, -20, __ATOMIC_ACQUIRE) != -5)
    abort ();

  if (__atomic_max_fetch (&sv, 10, __ATOMIC_RELEASE) != 10)
    abort ();

  if (__atomic_max_fetch (&sv, 50, __ATOMIC_ACQ_REL) != 50)
    abort ();

  if (__atomic_max_fetch (&sv, SHRT_MAX, __ATOMIC_SEQ_CST) != SHRT_MAX)
    abort ();

  if (sv != SHRT_MAX)
    abort ();
}

void
test_umax_fetch ()
{
  unsigned short uv = 10;

  if (__atomic_max_fetch (&uv, 50, __ATOMIC_RELAXED) != 50)
    abort ();

  if (__atomic_max_fetch (&uv, 50, __ATOMIC_CONSUME) != 50)
    abort ();

  if (__atomic_max_fetch (&uv, 30, __ATOMIC_ACQUIRE) != 50)
    abort ();

  if (__atomic_max_fetch (&uv, 100, __ATOMIC_RELEASE) != 100)
    abort ();

  if (__atomic_max_fetch (&uv, 200, __ATOMIC_ACQ_REL) != 200)
    abort ();

  if (__atomic_max_fetch (&uv, USHRT_MAX, __ATOMIC_SEQ_CST) != USHRT_MAX)
    abort ();

  if (uv != USHRT_MAX)
    abort ();
}

/* Test the OP routines with a result which isn't used. Use both variations
   within each function.  */

void
test_add ()
{
  v = 0;
  count = 1;

  __atomic_add_fetch (&v, count, __ATOMIC_RELAXED);
  if (v != 1)
    abort ();

  __atomic_fetch_add (&v, count, __ATOMIC_CONSUME);
  if (v != 2)
    abort ();

  __atomic_add_fetch (&v, 1 , __ATOMIC_ACQUIRE);
  if (v != 3)
    abort ();

  __atomic_fetch_add (&v, 1, __ATOMIC_RELEASE);
  if (v != 4)
    abort ();

  __atomic_add_fetch (&v, count, __ATOMIC_ACQ_REL);
  if (v != 5)
    abort ();

  __atomic_fetch_add (&v, count, __ATOMIC_SEQ_CST);
  if (v != 6)
    abort ();
}


void
test_sub()
{
  v = res = 20;
  count = 0;

  __atomic_sub_fetch (&v, count + 1, __ATOMIC_RELAXED);
  if (v != --res)
    abort ();

  __atomic_fetch_sub (&v, count + 1, __ATOMIC_CONSUME);
  if (v != --res)
    abort ();                                                  
                                                               
  __atomic_sub_fetch (&v, 1, __ATOMIC_ACQUIRE);
  if (v != --res)
    abort ();                                                  
                                                               
  __atomic_fetch_sub (&v, 1, __ATOMIC_RELEASE);
  if (v != --res)
    abort ();                                                  
                                                               
  __atomic_sub_fetch (&v, count + 1, __ATOMIC_ACQ_REL);
  if (v != --res)
    abort ();                                                  
                                                               
  __atomic_fetch_sub (&v, count + 1, __ATOMIC_SEQ_CST);
  if (v != --res)
    abort ();
}

void
test_and ()
{
  v = init;

  __atomic_and_fetch (&v, 0, __ATOMIC_RELAXED);
  if (v != 0)
    abort ();

  v = init;
  __atomic_fetch_and (&v, init, __ATOMIC_CONSUME);
  if (v != init)
    abort ();

  __atomic_and_fetch (&v, 0, __ATOMIC_ACQUIRE);
  if (v != 0)
    abort ();

  v = ~v;
  __atomic_fetch_and (&v, init, __ATOMIC_RELEASE);
  if (v != init)
    abort ();

  __atomic_and_fetch (&v, 0, __ATOMIC_ACQ_REL);
  if (v != 0)
    abort ();

  v = ~v;
  __atomic_fetch_and (&v, 0, __ATOMIC_SEQ_CST);
  if (v != 0)
    abort ();
}

void
test_nand ()
{
  v = init;

  __atomic_fetch_nand (&v, 0, __ATOMIC_RELAXED);
  if (v != init)
    abort ();

  __atomic_fetch_nand (&v, init, __ATOMIC_CONSUME);
  if (v != 0)
    abort ();

  __atomic_nand_fetch (&v, 0, __ATOMIC_ACQUIRE);
  if (v != init)
    abort ();

  __atomic_nand_fetch (&v, init, __ATOMIC_RELEASE);
  if (v != 0)
    abort ();

  __atomic_fetch_nand (&v, init, __ATOMIC_ACQ_REL);
  if (v != init)
    abort ();

  __atomic_nand_fetch (&v, 0, __ATOMIC_SEQ_CST);
  if (v != init)
    abort ();
}



void
test_xor ()
{
  v = init;
  count = 0;

  __atomic_xor_fetch (&v, count, __ATOMIC_RELAXED);
  if (v != init)
    abort ();

  __atomic_fetch_xor (&v, ~count, __ATOMIC_CONSUME);
  if (v != 0)
    abort ();

  __atomic_xor_fetch (&v, 0, __ATOMIC_ACQUIRE);
  if (v != 0)
    abort ();

  __atomic_fetch_xor (&v, ~count, __ATOMIC_RELEASE);
  if (v != init)
    abort ();

  __atomic_fetch_xor (&v, 0, __ATOMIC_ACQ_REL);
  if (v != init)
    abort ();

  __atomic_xor_fetch (&v, ~count, __ATOMIC_SEQ_CST);
  if (v != 0)
    abort ();
}

void
test_or ()
{
  v = 0;
  count = 1;

  __atomic_or_fetch (&v, count, __ATOMIC_RELAXED);
  if (v != 1)
    abort ();

  count *= 2;
  __atomic_fetch_or (&v, count, __ATOMIC_CONSUME);
  if (v != 3)
    abort ();

  count *= 2;
  __atomic_or_fetch (&v, 4, __ATOMIC_ACQUIRE);
  if (v != 7)
    abort ();

  count *= 2;
  __atomic_fetch_or (&v, 8, __ATOMIC_RELEASE);
  if (v != 15)
    abort ();

  count *= 2;
  __atomic_or_fetch (&v, count, __ATOMIC_ACQ_REL);
  if (v != 31)
    abort ();

  count *= 2;
  __atomic_fetch_or (&v, count, __ATOMIC_SEQ_CST);
  if (v != 63)
    abort ();
}

void
test_smin ()
{
  short sv = 10;

  __atomic_min_fetch (&sv, 5, __ATOMIC_RELAXED);
  if (sv != 5)
    abort ();

  __atomic_fetch_min (&sv, 5, __ATOMIC_CONSUME);
  if (sv != 5)
    abort ();

  __atomic_min_fetch (&sv, -10, __ATOMIC_ACQUIRE);
  if (sv != -10)
    abort ();

  __atomic_fetch_min (&sv, -20, __ATOMIC_RELEASE);
  if (sv != -20)
    abort ();

  __atomic_min_fetch (&sv, SHRT_MIN, __ATOMIC_ACQ_REL);
  if (sv != SHRT_MIN)
    abort ();

  __atomic_fetch_min (&sv, SHRT_MAX, __ATOMIC_SEQ_CST);

  if (sv != SHRT_MIN)
    abort ();
}

void
test_umin ()
{
  unsigned short uv = 100;

  __atomic_min_fetch (&uv, 50, __ATOMIC_RELAXED);
  if (uv != 50)
    abort ();

  __atomic_fetch_min (&uv, 50, __ATOMIC_CONSUME);
  if (uv != 50)
    abort ();

  __atomic_min_fetch (&uv, 75, __ATOMIC_ACQUIRE);
  if (uv != 50)
    abort ();

  __atomic_fetch_min (&uv, 10, __ATOMIC_RELEASE);
  if (uv != 10)
    abort ();

  __atomic_min_fetch (&uv, 0, __ATOMIC_ACQ_REL);
  if (uv != 0)
    abort ();

  __atomic_fetch_min (&uv, USHRT_MAX, __ATOMIC_SEQ_CST);

  if (uv != 0)
    abort ();
}

void
test_smax ()
{
  short sv = -10;

  __atomic_max_fetch (&sv, -5, __ATOMIC_RELAXED);
  if (sv != -5)
    abort ();

  __atomic_fetch_max (&sv, -5, __ATOMIC_CONSUME);
  if (sv != -5)
    abort ();

  __atomic_max_fetch (&sv, -20, __ATOMIC_ACQUIRE);
  if (sv != -5)
    abort ();

  __atomic_fetch_max (&sv, 10, __ATOMIC_RELEASE);
  if (sv != 10)
    abort ();

  __atomic_max_fetch (&sv, 50, __ATOMIC_ACQ_REL);
  if (sv != 50)
    abort ();

  __atomic_fetch_max (&sv, SHRT_MAX, __ATOMIC_SEQ_CST);

  if (sv != SHRT_MAX)
    abort ();
}

void
test_umax ()
{
  unsigned short uv = 10;

  __atomic_max_fetch (&uv, 50, __ATOMIC_RELAXED);
  if (uv != 50)
    abort ();

  __atomic_fetch_max (&uv, 50, __ATOMIC_CONSUME);
  if (uv != 50)
    abort ();

  __atomic_max_fetch (&uv, 30, __ATOMIC_ACQUIRE);
  if (uv != 50)
    abort ();

  __atomic_fetch_max (&uv, 100, __ATOMIC_RELEASE);
  if (uv != 100)
    abort ();

  __atomic_max_fetch (&uv, 200, __ATOMIC_ACQ_REL);
  if (uv != 200)
    abort ();

  __atomic_fetch_max (&uv, USHRT_MAX, __ATOMIC_SEQ_CST);

  if (uv != USHRT_MAX)
    abort ();
}

int
main ()
{
  test_fetch_add ();
  test_fetch_sub ();
  test_fetch_and ();
  test_fetch_nand ();
  test_fetch_xor ();
  test_fetch_or ();
  test_fetch_smin ();
  test_fetch_umin ();
  test_fetch_smax ();
  test_fetch_umax ();

  test_add_fetch ();
  test_sub_fetch ();
  test_and_fetch ();
  test_nand_fetch ();
  test_xor_fetch ();
  test_or_fetch ();
  test_smin_fetch ();
  test_umin_fetch ();
  test_smax_fetch ();
  test_umax_fetch ();

  test_add ();
  test_sub ();
  test_and ();
  test_nand ();
  test_xor ();
  test_or ();
  test_smin ();
  test_umin ();
  test_smax ();
  test_umax ();

  return 0;
}
