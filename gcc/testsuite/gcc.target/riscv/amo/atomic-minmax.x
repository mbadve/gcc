#include <stdint.h>

extern void abort(void);

#define TEST_FETCH_OP(TYPE, VAR, OP, INIT, ARG, EXPECTED_OLD, EXPECTED_NEW, MEM_ORDER) \
  do { \
    VAR = INIT; \
    TYPE old_val = __atomic_fetch_##OP(&VAR, ARG, MEM_ORDER); \
    TYPE new_val = VAR; \
    if (old_val != EXPECTED_OLD) { \
      abort(); \
    } \
    if (new_val != EXPECTED_NEW) { \
      abort(); \
    } \
  } while (0)

#define TEST_OP_FETCH(TYPE, VAR, OP, INIT, ARG, EXPECTED_OLD, EXPECTED_NEW, MEM_ORDER) \
  do { \
    VAR = INIT; \
    TYPE result = __atomic_##OP##_fetch(&VAR, ARG, MEM_ORDER); \
    TYPE new_val = VAR; \
    if (result != EXPECTED_NEW) { \
      abort(); \
    } \
    if (new_val != EXPECTED_NEW) { \
      abort(); \
    } \
  } while (0)

#define GEN_TYPE_TESTS(TYPE, VAR, SUFFIX, IS_SIGNED) \
  TYPE VAR; \
  void test_##SUFFIX() { \
    TEST_FETCH_OP(TYPE, VAR, min, 10, 5, 10, 5, __ATOMIC_RELAXED); \
    TEST_FETCH_OP(TYPE, VAR, min, 10, 20, 10, 10, __ATOMIC_SEQ_CST); \
    TEST_FETCH_OP(TYPE, VAR, max, 10, 20, 10, 20, __ATOMIC_ACQUIRE); \
    TEST_FETCH_OP(TYPE, VAR, max, 10, 5, 10, 10, __ATOMIC_RELEASE); \
    \
    TEST_OP_FETCH(TYPE, VAR, min, 10, 5, 10, 5, __ATOMIC_CONSUME); \
    TEST_OP_FETCH(TYPE, VAR, min, 10, 20, 10, 10, __ATOMIC_ACQ_REL); \
    TEST_OP_FETCH(TYPE, VAR, max, 10, 20, 10, 20, __ATOMIC_RELAXED); \
    TEST_OP_FETCH(TYPE, VAR, max, 10, 5, 10, 10, __ATOMIC_SEQ_CST); \
    \
    if (IS_SIGNED) { \
      TEST_FETCH_OP(TYPE, VAR, min, -10, -20, -10, -20, __ATOMIC_ACQUIRE); \
      TEST_FETCH_OP(TYPE, VAR, max, -10, 5, -10, 5, __ATOMIC_RELEASE); \
      TEST_FETCH_OP(TYPE, VAR, min, -5, -3, -5, -5, __ATOMIC_RELAXED); \
      TEST_FETCH_OP(TYPE, VAR, max, -20, -10, -20, -10, __ATOMIC_SEQ_CST); \
      TEST_OP_FETCH(TYPE, VAR, min, -100, 50, -100, -100, __ATOMIC_ACQ_REL); \
      TEST_OP_FETCH(TYPE, VAR, max, -50, -60, -50, -50, __ATOMIC_CONSUME); \
    } \
  }

GEN_TYPE_TESTS(int8_t, s8_var_test, s8, 1)
GEN_TYPE_TESTS(int16_t, s16_var_test, s16, 1)
GEN_TYPE_TESTS(int32_t, s32_var_test, s32, 1)
GEN_TYPE_TESTS(int64_t, s64_var_test, s64, 1)

GEN_TYPE_TESTS(uint8_t, u8_var_test, u8, 0)
GEN_TYPE_TESTS(uint16_t, u16_var_test, u16, 0)
GEN_TYPE_TESTS(uint32_t, u32_var_test, u32, 0)
GEN_TYPE_TESTS(uint64_t, u64_var_test, u64, 0)

void run_tests() {
  test_s8();
  test_s16();
  test_s32();
  test_s64();
  test_u8();
  test_u16();
  test_u32();
  test_u64();
}

int8_t s8_var = 0;

int8_t
test_smin_s8 (int8_t a)
{
  return __atomic_fetch_min (&s8_var, a, __ATOMIC_RELAXED);
}

int8_t
test_smax_s8 (int8_t a)
{
  return __atomic_fetch_max (&s8_var, a, __ATOMIC_RELEASE);
}

int16_t s16_var = 0;

int16_t
test_smin_s16 (int16_t a)
{
  return __atomic_fetch_min (&s16_var, a, __ATOMIC_ACQUIRE);
}

int16_t
test_smax_s16 (int16_t a)
{
  return __atomic_fetch_max (&s16_var, a, __ATOMIC_ACQ_REL);
}

int32_t s32_var = 0;

int32_t
test_smin_s32 (int32_t a)
{
  return __atomic_fetch_min (&s32_var, a, __ATOMIC_RELAXED);
}

int32_t
test_smax_s32 (int32_t a)
{
  return __atomic_fetch_max (&s32_var, a, __ATOMIC_SEQ_CST);
}


int64_t s64_var = 0;

int64_t
test_smin_s64 (int64_t a)
{
  return __atomic_fetch_min (&s64_var, a, __ATOMIC_ACQUIRE);
}

int64_t
test_smax_s64 (int64_t a)
{
  return __atomic_fetch_max (&s64_var, a, __ATOMIC_RELAXED);
}


uint8_t u8_var = 0;

uint8_t
test_umin_u8 (uint8_t a)
{
  return __atomic_fetch_min (&u8_var, a, __ATOMIC_RELAXED);
}

uint8_t
test_umax_u8 (uint8_t a)
{
  return __atomic_fetch_max (&u8_var, a, __ATOMIC_CONSUME);
}

uint16_t u16_var = 0;

uint16_t
test_umin_u16 (uint16_t a)
{
  return __atomic_fetch_min (&u16_var, a, __ATOMIC_ACQUIRE);
}

uint16_t
test_umax_u16 (uint16_t a)
{
  return __atomic_fetch_max (&u16_var, a, __ATOMIC_RELEASE);
}

uint32_t u32_var = 0;

uint32_t
test_umin_u32 (uint32_t a)
{
  return __atomic_fetch_min (&u32_var, a, __ATOMIC_ACQ_REL);
}

uint32_t
test_umax_u32 (uint32_t a)
{
  return __atomic_fetch_max (&u32_var, a, __ATOMIC_RELAXED);
}


uint64_t u64_var = 0;

uint64_t
test_umin_u64 (uint64_t a)
{
  return __atomic_fetch_min (&u64_var, a, __ATOMIC_RELAXED);
}

uint64_t
test_umax_u64 (uint64_t a)
{
  return __atomic_fetch_max (&u64_var, a, __ATOMIC_ACQ_REL);
}


