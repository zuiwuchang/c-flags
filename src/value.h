#ifndef PPP_C_FLAGS__VALUE_H
#define PPP_C_FLAGS__VALUE_H
#include "flags.h"

typedef struct
{
    PPP_C_FLAGS_BOOL b;
    PPP_C_FLAGS_INT8 i8;
    PPP_C_FLAGS_INT16 i16;
    PPP_C_FLAGS_INT32 i32;
    PPP_C_FLAGS_INT64 i64;
    PPP_C_FLAGS_UINT8 u8;
    PPP_C_FLAGS_UINT16 u16;
    PPP_C_FLAGS_UINT32 u32;
    PPP_C_FLAGS_UINT64 u64;
    PPP_C_FLAGS_FLOAT32 f32;
    PPP_C_FLAGS_FLOAT64 f64;
    PPP_C_FLAGS_STRING s;
} value_flags_t;

int init_value_command(ppp_c_flags_command_t *parent, value_flags_t *flags);

#endif