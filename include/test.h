#ifndef __TEST_H__
#define __TEST_H__

#include <stdio.h>

#define RED   "\x1B[31m"
#define GREEN "\x1B[32m"
#define RESET "\x1B[0m"

static unsigned count_of_tests = 0;
static unsigned count_of_assertions = 0;
static unsigned count_of_failed_tests = 0;

#define C_TEST(test_name) void test_name(void)
#define C_TEST_SUITE(test_suite_name) void test_suite_name(void)

#define C_SAFE_BLOCK(block) do { block } while (0)
 
#define ASSERT_EQ(f, s) \
C_SAFE_BLOCK \
( \
    count_of_assertions++; \
    if ((f) != (s)) \
    { \
        printf(RED); \
        printf("%s failed:\n\t%s:%d: %s == %s\n", __func__, __FILE__, __LINE__, #f, #s); \
        printf(RESET); \
        count_of_failed_tests++; \
        return; \
    } \
)

#define ASSERT_EQ_CMP(f, s, cmp) \
C_SAFE_BLOCK \
( \
    count_of_assertions++; \
    if (cmp((f), (s)) != 0) \
    { \
        printf(RED); \
        printf("%s failed:\n\t%s:%d: %s == %s\n", __func__, __FILE__, __LINE__, #f, #s); \
        printf(RESET); \
        count_of_failed_tests++; \
        return; \
    } \
)

#define ASSERT_NE(f, s) \
C_SAFE_BLOCK \
( \
    count_of_assertions++; \
    if ((f) == (s)) \
    { \
        printf(RED); \
        printf("%s failed:\n\t%s:%d: %s != %s\n", __func__, __FILE__, __LINE__, #f, #s); \
        printf(RESET); \
        count_of_failed_tests++; \
        return; \
    } \
)

#define ASSERT_NE_CMP(f, s, cmp) \
C_SAFE_BLOCK \
( \
    count_of_assertions++; \
    if (cmp((f), (s)) == 0) \
    { \
        printf(RED); \
        printf("%s failed:\n\t%s:%d: %s != %s\n", __func__, __FILE__, __LINE__, #f, #s); \
        printf(RESET); \
        count_of_failed_tests++; \
        return; \
    } \
)

#define ASSERT_TRUE(case) \
C_SAFE_BLOCK \
( \
    count_of_assertions++; \
    if (!(case)) \
    { \
        printf(RED); \
        printf("%s failed:\n\t%s:%d: %s\n", __func__, __FILE__, __LINE__, #case); \
        printf(RESET); \
        count_of_failed_tests++; \
        return; \
    } \
)

#define ASSERT_FALSE(case) \
C_SAFE_BLOCK \
( \
    count_of_assertions++; \
    if (case) \
    { \
        printf(RED); \
        printf("%s failed:\n\t%s:%d: %s\n", __func__, __FILE__, __LINE__, #case); \
        printf(RESET); \
        count_of_failed_tests++; \
        return; \
    } \
)

#define C_RUN_TEST(test_name) \
C_SAFE_BLOCK \
( \
    count_of_tests++; \
    test_name(); \
)

#define C_RUN_SUITE(test_suite_name) test_suite_name()

#define C_TEST_REPORT() \
C_SAFE_BLOCK \
( \
    printf("Test report:\n"); \
    printf("\t%u tests\n\t%u assertions\n", count_of_tests, count_of_assertions); \
    printf(count_of_failed_tests > 0 ? RED : GREEN); \
    printf("\t%u failed tests\n", count_of_failed_tests); \
    printf(RESET); \
)

#endif