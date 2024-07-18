#ifndef PPP_C_FLAGS__VALUE_ARRAY_H
#define PPP_C_FLAGS__VALUE_ARRAY_H
#include "flags.h"

typedef struct
{
    PPP_C_FLAGS_BOOL_ARRAY b;
    PPP_C_FLAGS_INT8_ARRAY i8;
    PPP_C_FLAGS_INT16_ARRAY i16;
    PPP_C_FLAGS_INT32_ARRAY i32;
    PPP_C_FLAGS_INT64_ARRAY i64;
    PPP_C_FLAGS_UINT8_ARRAY u8;
    PPP_C_FLAGS_UINT16_ARRAY u16;
    PPP_C_FLAGS_UINT32_ARRAY u32;
    PPP_C_FLAGS_UINT64_ARRAY u64;
    PPP_C_FLAGS_FLOAT32_ARRAY f32;
    PPP_C_FLAGS_FLOAT64_ARRAY f64;
    PPP_C_FLAGS_STRING_ARRAY s;
} value_array_flags_t;

int init_value_array_command(ppp_c_flags_command_t *parent, value_array_flags_t *flags);

#endif