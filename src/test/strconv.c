#include "strconv.h"
#include "../flags.h"
#include <string.h>

#define nil 0
#define ErrSyntax -1
#define ErrRange 1

typedef struct
{
    const char *in;
    uint64_t out;
    int err;
} parseUint64Test;
static parseUint64Test parseUint64Tests[] = {
    {"", 0, ErrSyntax},
    {"0", 0, nil},
    {"1", 1, nil},
    {"12345", 12345, nil},
    {"012345", 12345, nil},
    {"12345x", 0, ErrSyntax},
    {"98765432100", 98765432100, nil},
    {"18446744073709551615", 18446744073709551615UL, nil},
    {"18446744073709551616", 18446744073709551615UL, ErrRange},
    {"18446744073709551620", 18446744073709551615UL, ErrRange},
    {"1_2_3_4_5", 0, ErrSyntax}, // base=10 so no underscores allowed
    {"_12345", 0, ErrSyntax},
    {"1__2345", 0, ErrSyntax},
    {"12345_", 0, ErrSyntax},
    {"-0", 0, ErrSyntax},
    {"-1", 0, ErrSyntax},
    {"+1", 0, ErrSyntax},
};
typedef struct
{
    const char *in;
    int base;
    uint64_t out;
    int err;
} parseUint64BaseTest;
static parseUint64BaseTest parseUint64BaseTests[] = {
    {"", 0, 0, ErrSyntax},
    {"0", 0, 0, nil},
    {"0x", 0, 0, ErrSyntax},
    {"0X", 0, 0, ErrSyntax},
    {"1", 0, 1, nil},
    {"12345", 0, 12345, nil},
    {"012345", 0, 012345, nil},
    {"0x12345", 0, 0x12345, nil},
    {"0X12345", 0, 0x12345, nil},
    {"12345x", 0, 0, ErrSyntax},
    {"0xabcdefg123", 0, 0, ErrSyntax},
    {"123456789abc", 0, 0, ErrSyntax},
    {"98765432100", 0, 98765432100, nil},
    {"18446744073709551615", 0, 18446744073709551615UL, nil},
    {"18446744073709551616", 0, 18446744073709551615UL, ErrRange},
    {"18446744073709551620", 0, 18446744073709551615UL, ErrRange},
    {"0xFFFFFFFFFFFFFFFF", 0, 18446744073709551615UL, nil},
    {"0x10000000000000000", 0, 18446744073709551615UL, ErrRange},
    {"01777777777777777777777", 0, 18446744073709551615UL, nil},
    {"01777777777777777777778", 0, 0, ErrSyntax},
    {"02000000000000000000000", 0, 18446744073709551615UL, ErrRange},
    {"0200000000000000000000", 0, ((uint64_t)1) << 61, nil},
    {"0b", 0, 0, ErrSyntax},
    {"0B", 0, 0, ErrSyntax},
    {"0b101", 0, 5, nil},
    {"0B101", 0, 5, nil},
    {"0o", 0, 0, ErrSyntax},
    {"0O", 0, 0, ErrSyntax},
    {"0o377", 0, 255, nil},
    {"0O377", 0, 255, nil},

    // underscores allowed with base == 0 only
    {"1_2_3_4_5", 0, 12345, nil}, // base 0 => 10
    {"_12345", 0, 0, ErrSyntax},
    {"1__2345", 0, 0, ErrSyntax},
    {"12345_", 0, 0, ErrSyntax},

    {"1_2_3_4_5", 10, 0, ErrSyntax}, // base 10
    {"_12345", 10, 0, ErrSyntax},
    {"1__2345", 10, 0, ErrSyntax},
    {"12345_", 10, 0, ErrSyntax},

    {"0x_1_2_3_4_5", 0, 0x12345, nil}, // base 0 => 16
    {"_0x12345", 0, 0, ErrSyntax},
    {"0x__12345", 0, 0, ErrSyntax},
    {"0x1__2345", 0, 0, ErrSyntax},
    {"0x1234__5", 0, 0, ErrSyntax},
    {"0x12345_", 0, 0, ErrSyntax},

    {"1_2_3_4_5", 16, 0, ErrSyntax}, // base 16
    {"_12345", 16, 0, ErrSyntax},
    {"1__2345", 16, 0, ErrSyntax},
    {"1234__5", 16, 0, ErrSyntax},
    {"12345_", 16, 0, ErrSyntax},

    {"0_1_2_3_4_5", 0, 012345, nil}, // base 0 => 8 (0377)
    {"_012345", 0, 0, ErrSyntax},
    {"0__12345", 0, 0, ErrSyntax},
    {"01234__5", 0, 0, ErrSyntax},
    {"012345_", 0, 0, ErrSyntax},

    {"0o_1_2_3_4_5", 0, 012345, nil}, // base 0 => 8 (0o377)
    {"_0o12345", 0, 0, ErrSyntax},
    {"0o__12345", 0, 0, ErrSyntax},
    {"0o1234__5", 0, 0, ErrSyntax},
    {"0o12345_", 0, 0, ErrSyntax},

    {"0_1_2_3_4_5", 8, 0, ErrSyntax}, // base 8
    {"_012345", 8, 0, ErrSyntax},
    {"0__12345", 8, 0, ErrSyntax},
    {"01234__5", 8, 0, ErrSyntax},
    {"012345_", 8, 0, ErrSyntax},

    {"0b_1_0_1", 0, 5, nil}, // base 0 => 2 (0b101)
    {"_0b101", 0, 0, ErrSyntax},
    {"0b__101", 0, 0, ErrSyntax},
    {"0b1__01", 0, 0, ErrSyntax},
    {"0b10__1", 0, 0, ErrSyntax},
    {"0b101_", 0, 0, ErrSyntax},

    {"1_0_1", 2, 0, ErrSyntax}, // base 2
    {"_101", 2, 0, ErrSyntax},
    {"1_01", 2, 0, ErrSyntax},
    {"10_1", 2, 0, ErrSyntax},
    {"101_", 2, 0, ErrSyntax},
};

typedef struct
{
    const char *in;
    int64_t out;
    int err;
} parseInt64Test;

static parseInt64Test parseInt64Tests[] = {
    {"", 0, ErrSyntax},
    {"0", 0, nil},
    {"-0", 0, nil},
    {"+0", 0, nil},
    {"1", 1, nil},
    {"-1", -1, nil},
    {"+1", 1, nil},
    {"12345", 12345, nil},
    {"-12345", -12345, nil},
    {"012345", 12345, nil},
    {"-012345", -12345, nil},
    {"98765432100", 98765432100, nil},
    {"-98765432100", -98765432100, nil},
    {"9223372036854775807", 9223372036854775807, nil},
    {"-9223372036854775807", -9223372036854775807, nil},
    {"9223372036854775808", 9223372036854775807, ErrRange},
    {"-9223372036854775808", -9223372036854775808UL, nil},
    {"9223372036854775809", 9223372036854775807, ErrRange},
    {"-9223372036854775809", -9223372036854775808UL, ErrRange},
    {"-1_2_3_4_5", 0, ErrSyntax}, // base=10 so no underscores allowed
    {"-_12345", 0, ErrSyntax},
    {"_12345", 0, ErrSyntax},
    {"1__2345", 0, ErrSyntax},
    {"12345_", 0, ErrSyntax},
    {"123%45", 0, ErrSyntax},
};
typedef struct
{
    const char *in;
    int base;
    int64_t out;
    int err;
} parseInt64BaseTest;
static parseInt64BaseTest parseInt64BaseTests[] = {
    {"", 0, 0, ErrSyntax},
    {"0", 0, 0, nil},
    {"-0", 0, 0, nil},
    {"1", 0, 1, nil},
    {"-1", 0, -1, nil},
    {"12345", 0, 12345, nil},
    {"-12345", 0, -12345, nil},
    {"012345", 0, 012345, nil},
    {"-012345", 0, -012345, nil},
    {"0x12345", 0, 0x12345, nil},
    {"-0X12345", 0, -0x12345, nil},
    {"12345x", 0, 0, ErrSyntax},
    {"-12345x", 0, 0, ErrSyntax},
    {"98765432100", 0, 98765432100, nil},
    {"-98765432100", 0, -98765432100, nil},
    {"9223372036854775807", 0, 9223372036854775807, nil},
    {"-9223372036854775807", 0, -(9223372036854775807), nil},
    {"9223372036854775808", 0, 9223372036854775807, ErrRange},
    {"-9223372036854775808", 0, ((int64_t)-1) << 63, nil},
    {"9223372036854775809", 0, 9223372036854775807, ErrRange},
    {"-9223372036854775809", 0, ((int64_t)-1) << 63, ErrRange},

    // other bases
    {"g", 17, 16, nil},
    {"10", 25, 25, nil},
    {"holycow", 35, ((((((int64_t)(17 * 35 + 24)) * 35 + 21) * 35 + 34) * 35 + 12) * 35 + 24) * 35 + 32, nil},
    {"holycow", 36, ((((((int64_t)(17 * 36 + 24)) * 36 + 21) * 36 + 34) * 36 + 12) * 36 + 24) * 36 + 32, nil},

    // base 2
    {"0", 2, 0, nil},
    {"-1", 2, -1, nil},
    {"1010", 2, 10, nil},
    {"1000000000000000", 2, 1 << 15, nil},
    {"111111111111111111111111111111111111111111111111111111111111111", 2, 9223372036854775807, nil},
    {"1000000000000000000000000000000000000000000000000000000000000000", 2, 9223372036854775807, ErrRange},
    {"-1000000000000000000000000000000000000000000000000000000000000000", 2, ((int64_t)-1) << 63, nil},
    {"-1000000000000000000000000000000000000000000000000000000000000001", 2, ((int64_t)-1) << 63, ErrRange},

    // base 8
    {"-10", 8, -8, nil},
    {"57635436545", 8, 057635436545, nil},
    {"100000000", 8, 1 << 24, nil},

    // base 16
    {"10", 16, 16, nil},
    {"-123456789abcdef", 16, -0x123456789abcdef, nil},
    {"7fffffffffffffff", 16, 9223372036854775807, nil},

    // underscores
    {"-0x_1_2_3_4_5", 0, -0x12345, nil},
    {"0x_1_2_3_4_5", 0, 0x12345, nil},
    {"-_0x12345", 0, 0, ErrSyntax},
    {"_-0x12345", 0, 0, ErrSyntax},
    {"_0x12345", 0, 0, ErrSyntax},
    {"0x__12345", 0, 0, ErrSyntax},
    {"0x1__2345", 0, 0, ErrSyntax},
    {"0x1234__5", 0, 0, ErrSyntax},
    {"0x12345_", 0, 0, ErrSyntax},

    {"-0_1_2_3_4_5", 0, -012345, nil}, // octal
    {"0_1_2_3_4_5", 0, 012345, nil},   // octal
    {"-_012345", 0, 0, ErrSyntax},
    {"_-012345", 0, 0, ErrSyntax},
    {"_012345", 0, 0, ErrSyntax},
    {"0__12345", 0, 0, ErrSyntax},
    {"01234__5", 0, 0, ErrSyntax},
    {"012345_", 0, 0, ErrSyntax},

    {"+0xf", 0, 0xf, nil},
    {"-0xf", 0, -0xf, nil},
    {"0x+f", 0, 0, ErrSyntax},
    {"0x-f", 0, 0, ErrSyntax},
};
typedef struct
{
    const char *in;
    uint32_t out;
    int err;
} parseUint32Test;
static parseUint32Test parseUint32Tests[] = {
    {"", 0, ErrSyntax},
    {"0", 0, nil},
    {"1", 1, nil},
    {"12345", 12345, nil},
    {"012345", 12345, nil},
    {"12345x", 0, ErrSyntax},
    {"987654321", 987654321, nil},
    {"4294967295", 4294967295, nil},
    {"4294967296", 4294967295, ErrRange},
    {"1_2_3_4_5", 0, ErrSyntax}, // base=10 so no underscores allowed
    {"_12345", 0, ErrSyntax},
    {"_12345", 0, ErrSyntax},
    {"1__2345", 0, ErrSyntax},
    {"12345_", 0, ErrSyntax},
};
typedef struct
{
    const char *in;
    int32_t out;
    int err;
} parseInt32Test;
static parseInt32Test parseInt32Tests[] = {
    {"", 0, ErrSyntax},
    {"0", 0, nil},
    {"-0", 0, nil},
    {"1", 1, nil},
    {"-1", -1, nil},
    {"12345", 12345, nil},
    {"-12345", -12345, nil},
    {"012345", 12345, nil},
    {"-012345", -12345, nil},
    {"12345x", 0, ErrSyntax},
    {"-12345x", 0, ErrSyntax},
    {"987654321", 987654321, nil},
    {"-987654321", -987654321, nil},
    {"2147483647", 2147483647, nil},
    {"-2147483647", -(2147483647), nil},
    {"2147483648", 2147483647, ErrRange},
    {"-2147483648", -2147483648, nil},
    {"2147483649", 2147483647, ErrRange},
    {"-2147483649", -2147483648, ErrRange},
    {"-1_2_3_4_5", 0, ErrSyntax}, // base=10 so no underscores allowed
    {"-_12345", 0, ErrSyntax},
    {"_12345", 0, ErrSyntax},
    {"1__2345", 0, ErrSyntax},
    {"12345_", 0, ErrSyntax},
    {"123%45", 0, ErrSyntax},
};

static TESTS_GROUP_FUNC(strconv, parse_uint32, t)
{
    parseUint32Test test;
    size_t count = sizeof(parseUint32Tests) / sizeof(parseUint32Test);
    int err;
    for (size_t i = 0; i < count; i++)
    {
        test = parseUint32Tests[i];
        uint64_t v = 0;
        err = ppp_c_flags_parse_uint64(
            test.in, strlen(test.in),
            10, 32,
            &v);
        CuAssertUint64Equals_Msg(
            t,
            test.in,
            test.out, v);
        CuAssertIntEquals_Msg(
            t,
            test.in,
            test.err, err);
    }
}
static TESTS_GROUP_FUNC(strconv, parse_uint64, t)
{
    parseUint64Test test;
    size_t count = sizeof(parseUint64Tests) / sizeof(parseUint64Test);
    int err;
    for (size_t i = 0; i < count; i++)
    {
        test = parseUint64Tests[i];
        uint64_t v = 0;
        err = ppp_c_flags_parse_uint64(
            test.in, strlen(test.in),
            10, 64,
            &v);
        CuAssertUint64Equals_Msg(
            t,
            test.in,
            test.out, v);
        CuAssertIntEquals_Msg(
            t,
            test.in,
            test.err, err);
    }
}
static TESTS_GROUP_FUNC(strconv, parse_uint64_base, t)
{
    parseUint64BaseTest test;
    size_t count = sizeof(parseUint64BaseTests) / sizeof(parseUint64BaseTest);
    int err;
    for (size_t i = 0; i < count; i++)
    {
        test = parseUint64BaseTests[i];
        uint64_t v = 0;
        err = ppp_c_flags_parse_uint64(
            test.in, strlen(test.in),
            test.base, 64,
            &v);
        CuAssertUint64Equals_Msg(
            t,
            test.in,
            test.out, v);
        CuAssertIntEquals_Msg(
            t,
            test.in,
            test.err, err);
    }
}
static TESTS_GROUP_FUNC(strconv, parse_int32, t)
{
    parseInt32Test test;
    size_t count = sizeof(parseInt32Tests) / sizeof(parseInt32Test);
    int err;
    for (size_t i = 0; i < count; i++)
    {
        test = parseInt32Tests[i];
        int64_t v = 0;
        err = ppp_c_flags_parse_int64(
            test.in, strlen(test.in),
            10, 32,
            &v);
        CuAssertUint64Equals_Msg(
            t,
            test.in,
            test.out, v);
        CuAssertIntEquals_Msg(
            t,
            test.in,
            test.err, err);
    }
}
static TESTS_GROUP_FUNC(strconv, parse_int64, t)
{
    parseInt64Test test;
    size_t count = sizeof(parseInt64Tests) / sizeof(parseInt64Test);
    int err;
    for (size_t i = 0; i < count; i++)
    {
        test = parseInt64Tests[i];
        int64_t v = 0;
        err = ppp_c_flags_parse_int64(
            test.in, strlen(test.in),
            10, 64,
            &v);
        CuAssertUint64Equals_Msg(
            t,
            test.in,
            test.out, v);
        CuAssertIntEquals_Msg(
            t,
            test.in,
            test.err, err);
    }
}
static TESTS_GROUP_FUNC(strconv, parse_int64_base, t)
{
    parseInt64BaseTest test;
    size_t count = sizeof(parseInt64BaseTests) / sizeof(parseInt64BaseTest);
    int err;
    for (size_t i = 0; i < count; i++)
    {
        test = parseInt64BaseTests[i];
        int64_t v = 0;
        err = ppp_c_flags_parse_int64(
            test.in, strlen(test.in),
            test.base, 64,
            &v);
        CuAssertUint64Equals_Msg(
            t,
            test.in,
            test.out, v);
        CuAssertIntEquals_Msg(
            t,
            test.in,
            test.err, err);
    }
}
TESTS_GROUP(suite, strconv)
{
    TESTS_GROUP_ADD_FUNC(suite, strconv, parse_uint32);
    TESTS_GROUP_ADD_FUNC(suite, strconv, parse_uint64);
    TESTS_GROUP_ADD_FUNC(suite, strconv, parse_uint64_base);

    TESTS_GROUP_ADD_FUNC(suite, strconv, parse_int32);
    TESTS_GROUP_ADD_FUNC(suite, strconv, parse_int64);
    TESTS_GROUP_ADD_FUNC(suite, strconv, parse_int64_base);
}
#undef nil
#undef ErrSyntax
#undef ErrRange