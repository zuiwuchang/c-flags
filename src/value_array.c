#include "value_array.h"
#include <stdio.h>
#define PRINT_FLAG_DEFAULT_ARRAY(arrs, format) \
    for (size_t i = 0; i < arrs->len; i++)     \
    {                                          \
        if (i)                                 \
            printf(", " format, arrs->p[i]);   \
        else                                   \
            printf(format, arrs->p[i]);        \
    }

#define PRINT_FLAG_ARRAY(arrs, name, format)    \
    if (arrs.len)                               \
    {                                           \
        printf("  " #name "=[");                \
        for (size_t i = 0; i < arrs.len; i++)   \
        {                                       \
            if (i)                              \
                printf(", " format, arrs.p[i]); \
            else                                \
                printf(format, arrs.p[i]);      \
        }                                       \
        puts("]");                              \
    }                                           \
    else                                        \
        puts("  " #name "=[]")

#define ADD_ARRAY_FLAG_RANGE(cmd, err, type, arrs_type, name, arrs, min, max) \
    static type name[] = {min, max};                                          \
    arrs.p = name;                                                            \
    arrs.len = sizeof(name) / sizeof(type);                                   \
    flag = ppp_c_flags_add_flag(                                              \
        cmd,                                                                  \
        #name, 0,                                                             \
        0,                                                                    \
        &arrs, arrs_type,                                                     \
        &err);                                                                \
    if (!flag)                                                                \
    {                                                                         \
        printf("Add flags " #name " fail: %s\n", ppp_c_flags_error(err));     \
        return -1;                                                            \
    }
static int print_float_value(struct ppp_c_flags_flag *flag, uint8_t value_type, void *value)
{
    switch (value_type)
    {
    case PPP_C_FLAGS_TYPE_FLOAT32_ARRAY:
    {
        PPP_C_FLAGS_FLOAT32_ARRAY *arrs = value;
        PRINT_FLAG_DEFAULT_ARRAY(arrs, "%g")
    }
    break;
    case PPP_C_FLAGS_TYPE_FLOAT64_ARRAY:
    {
        PPP_C_FLAGS_FLOAT64_ARRAY *arrs = value;
        PRINT_FLAG_DEFAULT_ARRAY(arrs, "%g")
    }
    break;
    }
}
static int value_array_handler(ppp_c_flags_command_t *command, int argc, char **argv, void *userdata)
{
    printf("value_array_handler:\n");
    value_array_flags_t *flags = userdata;

    printf("  b=[");
    for (size_t i = 0; i < flags->b.len; i++)
    {
        if (i)
            printf(",");
        printf("%s", flags->b.p[i] ? "true" : "false");
    }
    puts("]");

    PRINT_FLAG_ARRAY(flags->i8, i8, "%d");
    PRINT_FLAG_ARRAY(flags->i16, i16, "%d");
    PRINT_FLAG_ARRAY(flags->i32, i32, "%d");
    PRINT_FLAG_ARRAY(flags->i64, i64, "%ld");

    PRINT_FLAG_ARRAY(flags->u8, u8, "%u");
    PRINT_FLAG_ARRAY(flags->u16, u16, "%u");
    PRINT_FLAG_ARRAY(flags->u32, u32, "%u");
    PRINT_FLAG_ARRAY(flags->u64, u64, "%lu");

    PRINT_FLAG_ARRAY(flags->f32, f32, "%g");
    PRINT_FLAG_ARRAY(flags->f64, f64, "%g");

    PRINT_FLAG_ARRAY(flags->s, s, "%s");

    if (argc)
    {
        printf("args:\n");
        for (int i = 0; i < argc; i++)
        {
            printf("  %3d. %s\n", i + 1, argv[i]);
        }
    }
    return 0;
}
int init_value_array_command(ppp_c_flags_command_t *parent, value_array_flags_t *flags)
{
    int err = 0;
    ppp_c_flags_command_t *cmd = ppp_c_flags_add_command(
        parent,
        "array", "array example",
        value_array_handler, flags,
        &err);
    if (!cmd)
    {
        printf("Create array command fail: %s\n", ppp_c_flags_error(err));
        return -1;
    }

    static PPP_C_FLAGS_BOOL b[] = {0, 1};
    flags->b.p = b;
    flags->b.len = sizeof(b) / sizeof(PPP_C_FLAGS_BOOL);
    if (!ppp_c_flags_add_flag(
            cmd,
            "bool", 'b',
            0,
            &flags->b, PPP_C_FLAGS_TYPE_BOOL_ARRAY,
            &err))
    {
        printf("Add flags fail: %s\n", ppp_c_flags_error(err));
        return -1;
    }
    ppp_c_flags_flag_t *flag;
    ADD_ARRAY_FLAG_RANGE(
        cmd, err,
        PPP_C_FLAGS_INT8,
        PPP_C_FLAGS_TYPE_INT8_ARRAY,
        i8, flags->i8,
        -128, 127)
    ADD_ARRAY_FLAG_RANGE(
        cmd, err,
        PPP_C_FLAGS_INT16,
        PPP_C_FLAGS_TYPE_INT16_ARRAY,
        i16, flags->i16,
        -32768, 32767)
    ADD_ARRAY_FLAG_RANGE(
        cmd, err,
        PPP_C_FLAGS_INT32,
        PPP_C_FLAGS_TYPE_INT32_ARRAY,
        i32, flags->i32,
        -2147483648, 2147483647)
    ADD_ARRAY_FLAG_RANGE(
        cmd, err,
        PPP_C_FLAGS_INT64,
        PPP_C_FLAGS_TYPE_INT64_ARRAY,
        i64, flags->i64,
        -9223372036854775808UL, 9223372036854775807UL)
    ADD_ARRAY_FLAG_RANGE(
        cmd, err,
        PPP_C_FLAGS_UINT8,
        PPP_C_FLAGS_TYPE_UINT8_ARRAY,
        u8, flags->u8,
        0, 255)
    ADD_ARRAY_FLAG_RANGE(
        cmd, err,
        PPP_C_FLAGS_UINT16,
        PPP_C_FLAGS_TYPE_UINT16_ARRAY,
        u16, flags->u16,
        0, 65535)
    ADD_ARRAY_FLAG_RANGE(
        cmd, err,
        PPP_C_FLAGS_UINT32,
        PPP_C_FLAGS_TYPE_UINT32_ARRAY,
        u32, flags->u32,
        0, 4294967295)
    ADD_ARRAY_FLAG_RANGE(
        cmd, err,
        PPP_C_FLAGS_UINT64,
        PPP_C_FLAGS_TYPE_UINT64_ARRAY,
        u64, flags->u64,
        0, 18446744073709551615UL)

    ADD_ARRAY_FLAG_RANGE(
        cmd, err,
        PPP_C_FLAGS_FLOAT32,
        PPP_C_FLAGS_TYPE_FLOAT32_ARRAY,
        f32, flags->f32,
        3.40282346638528859811704183484516925440e+38, 1.401298464324817070923729583289916131280e-45)
    flag->print = print_float_value;
    ADD_ARRAY_FLAG_RANGE(
        cmd, err,
        PPP_C_FLAGS_FLOAT64,
        PPP_C_FLAGS_TYPE_FLOAT64_ARRAY,
        f64, flags->f64,
        1.79769313486231570814527423731704356798070e+308, 4.9406564584124654417656879286822137236505980e-324)
    flag->print = print_float_value;

    ADD_ARRAY_FLAG_RANGE(
        cmd, err,
        PPP_C_FLAGS_STRING,
        PPP_C_FLAGS_TYPE_STRING_ARRAY,
        s, flags->s,
        "abc", "中文測試")

    return 0;
}