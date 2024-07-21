#include "value.h"
#include <stdio.h>
static int value_handler(ppp_c_flags_command_t *command, int argc, char **argv, void *userdata)
{
    value_flags_t *flags = userdata;
    printf(
        "value_handler:\n"
        "  b=%s\n"
        "  i8=%d\n  i16=%d\n  i32=%d\n  i64=%ld\n"
        "  u8=%u\n  u16=%u\n  u32=%u\n  u64=%lu\n"
        "  f32=%g\n  f64=%g\n"
        "  s=%s\n",
        flags->b ? "true" : "false",
        flags->i8, flags->i16, flags->i32, flags->i64,
        flags->u8, flags->u16, flags->u32, flags->u64,
        flags->f32, flags->f64,
        flags->s);
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
int init_value_command(ppp_c_flags_command_t *parent, value_flags_t *flags)
{
    int err = 0;
    ppp_c_flags_command_t *cmd = ppp_c_flags_add_command(
        parent,
        "value", "value range example",
        value_handler, flags,
        &err);
    if (!cmd)
    {
        printf("Create value command fail: %s\n", ppp_c_flags_error(err));
        return -1;
    }
    if (!ppp_c_flags_add_flag(
            cmd,
            "bool", 'b',
            "boolean [1,T,true,TRUE,0,F,false,False]",
            &flags->b, PPP_C_FLAGS_TYPE_BOOL,
            &err))
    {
        printf("Add flags fail: %s\n", ppp_c_flags_error(err));
        return -1;
    }
    if (!ppp_c_flags_add_flag(
            cmd,
            "i8", 0,
            "int8 [-128,127]",
            &flags->i8, PPP_C_FLAGS_TYPE_INT8,
            &err))
    {
        printf("Add flags fail: %s\n", ppp_c_flags_error(err));
        return -1;
    }
    if (!ppp_c_flags_add_flag(
            cmd,
            "i16", 0,
            "int16 [-32768,32767]",
            &flags->i16, PPP_C_FLAGS_TYPE_INT16,
            &err))
    {
        printf("Add flags fail: %s\n", ppp_c_flags_error(err));
        return -1;
    }
    if (!ppp_c_flags_add_flag(
            cmd,
            "i32", 0,
            "int32 [-2147483648,2147483647]",
            &flags->i32, PPP_C_FLAGS_TYPE_INT32,
            &err))
    {
        printf("Add flags fail: %s\n", ppp_c_flags_error(err));
        return -1;
    }
    if (!ppp_c_flags_add_flag(
            cmd,
            "i64", 0,
            "int64 [-9223372036854775808,9223372036854775807]",
            &flags->i64, PPP_C_FLAGS_TYPE_INT64,
            &err))
    {
        printf("Add flags fail: %s\n", ppp_c_flags_error(err));
        return -1;
    }
    if (!ppp_c_flags_add_flag(
            cmd,
            "u8", 0,
            "uint8 [0,255]",
            &flags->u8, PPP_C_FLAGS_TYPE_UINT8,
            &err))
    {
        printf("Add flags fail: %s\n", ppp_c_flags_error(err));
        return -1;
    }
    if (!ppp_c_flags_add_flag(
            cmd,
            "u16", 0,
            "uint16 [0,65535]",
            &flags->u16, PPP_C_FLAGS_TYPE_UINT16,
            &err))
    {
        printf("Add flags fail: %s\n", ppp_c_flags_error(err));
        return -1;
    }
    if (!ppp_c_flags_add_flag(
            cmd,
            "u32", 0,
            "uint32 [0,4294967295]",
            &flags->u32, PPP_C_FLAGS_TYPE_UINT32,
            &err))
    {
        printf("Add flags fail: %s\n", ppp_c_flags_error(err));
        return -1;
    }
    if (!ppp_c_flags_add_flag(
            cmd,
            "u64", 0,
            "uint64 [0,18446744073709551615]",
            &flags->u64, PPP_C_FLAGS_TYPE_UINT64,
            &err))
    {
        printf("Add flags fail: %s\n", ppp_c_flags_error(err));
        return -1;
    }
    if (!ppp_c_flags_add_flag(
            cmd,
            "f32", 0,
            "float32 [3.40282346638528859811704183484516925440e+38,1.401298464324817070923729583289916131280e-45]",
            &flags->f32, PPP_C_FLAGS_TYPE_FLOAT32,
            &err))
    {
        printf("Add flags fail: %s\n", ppp_c_flags_error(err));
        return -1;
    }
    if (!ppp_c_flags_add_flag(
            cmd,
            "f64", 0,
            "float64 [1.79769313486231570814527423731704356798070e+308,4.9406564584124654417656879286822137236505980e-324]",
            &flags->f64, PPP_C_FLAGS_TYPE_FLOAT64,
            &err))
    {
        printf("Add flags fail: %s\n", ppp_c_flags_error(err));
        return -1;
    }
    if (!ppp_c_flags_add_flag(
            cmd,
            "s", 0,
            "any string",
            &flags->s, PPP_C_FLAGS_TYPE_STRING,
            &err))
    {
        printf("Add flags fail: %s\n", ppp_c_flags_error(err));
        return -1;
    }
    return 0;
}