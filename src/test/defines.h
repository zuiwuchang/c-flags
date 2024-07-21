#ifndef PPP_C_FLAGS__TESTS_DEFINES_H
#define PPP_C_FLAGS__TESTS_DEFINES_H
#include "../../third_party_repo/cutest/CuTest.h"
#include <stdint.h>

#define TESTS_ADD_GROUP(suite, name) unit_tests_##name(suite)

#define FUNC_NAME(name) _##name

#define TESTS_GROUP(suite, name) void unit_tests_##name(CuSuite *suite)
#define TESTS_GROUP_ADD_FUNC(suite, group, name) CuSuiteAdd(suite, CuTestNew(#group ".test_" #name, unit_tests__func_##group##__test_##name))
#define TESTS_GROUP_FUNC(group, name, t) void unit_tests__func_##group##__test_##name(CuTest *t)

void CuAssertIntUint64Equals_LineMsg(CuTest *tc, const char *file, int line, const char *message,
                                     uint64_t expected, uint64_t actual);
#define CuAssertUint64Equals_Msg(tc, ms, ex, ac) CuAssertIntUint64Equals_LineMsg((tc), __FILE__, __LINE__, (ms), (ex), (ac))

#endif