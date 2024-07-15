#include "flags.h"
#include "string.h"
#include <stdio.h>
#include <errno.h>
typedef struct
{
    const char *name;
    size_t name_len;

    void (*print_default)(ppp_c_flags_flag_t *flag);
    int (*set_flag)(ppp_c_flags_flag_t *flag, const char *s, size_t s_len);
} ppp_c_flags_flag_handler_t;
static void ppp_c_flags_handler_bool_print_default(ppp_c_flags_flag_t *flag)
{
    if (*(PPP_C_FLAGS_BOOL *)(&flag->_default))
    {
        printf("<default: true>");
    }
}
static int ppp_c_flags_handler_bool_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len)
{
    switch (ppp_c_flags_parse_bool(s, s_len))
    {
    case 0:
        *(PPP_C_FLAGS_BOOL *)flag->_value = 0;
        return 0;
    case 1:
        *(PPP_C_FLAGS_BOOL *)flag->_value = 1;
        return 0;
    default:
        return -1;
    }
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_bool = {
    .name = "bool",
    .name_len = 4,
    .print_default = ppp_c_flags_handler_bool_print_default,
    .set_flag = ppp_c_flags_handler_bool_set_flag,
};
static void ppp_c_flags_handler_int64_print_default(ppp_c_flags_flag_t *flag)
{
    PPP_C_FLAGS_INT64 val = *(PPP_C_FLAGS_INT64 *)(&flag->_default);
    if (val)
    {
        printf("<value: %ld>", val);
    }
}
static int ppp_c_flags_handler_int64_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len)
{
    return ppp_c_flags_parse_int64(
        s, s_len,
        0, 0,
        flag->_value);
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_int64 = {
    .name = "int64",
    .name_len = 5,
    .print_default = ppp_c_flags_handler_int64_print_default,
    .set_flag = ppp_c_flags_handler_int64_set_flag,
};
static void ppp_c_flags_handler_uint64_print_default(ppp_c_flags_flag_t *flag)
{
    PPP_C_FLAGS_UINT64 val = *(PPP_C_FLAGS_UINT64 *)(&flag->_default);
    if (val)
    {
        printf("<default: %lu>", val);
    }
}
static int ppp_c_flags_handler_uint64_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len)
{
    return ppp_c_flags_parse_uint64(
        s, s_len,
        0, 0,
        flag->_value);
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_uint64 = {
    .name = "uint64",
    .name_len = 6,
    .print_default = ppp_c_flags_handler_uint64_print_default,
    .set_flag = ppp_c_flags_handler_uint64_set_flag,
};
static void ppp_c_flags_handler_float64_print_default(ppp_c_flags_flag_t *flag)
{
    PPP_C_FLAGS_FLOAT64 val = *(PPP_C_FLAGS_FLOAT64 *)(&flag->_default);
    if (val)
    {
        printf("<default: %g>", val);
    }
}
static int ppp_c_flags_handler_float64_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len)
{
    int err = errno;
    errno = 0;
    char *end = 0;
    double v = strtod(s, &end);
    if (errno)
    {
        return -1;
    }
    errno = err;
    if (end && end[0] != 0)
    {
        return -1;
    }
    *(PPP_C_FLAGS_FLOAT64 *)flag->_value = v;
    return 0;
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_float64 = {
    .name = "float64",
    .name_len = 7,
    .print_default = ppp_c_flags_handler_float64_print_default,
    .set_flag = ppp_c_flags_handler_float64_set_flag,
};
static void ppp_c_flags_handler_string_print_default(ppp_c_flags_flag_t *flag)
{
    PPP_C_FLAGS_STRING s = *(PPP_C_FLAGS_STRING *)(&flag->_default);
    if (s && s[0] != 0)
    {
        printf("<default: %s>", s);
    }
}
static int ppp_c_flags_handler_string_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len)
{
    *(PPP_C_FLAGS_STRING *)flag->_value = s;
    return 0;
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_string = {
    .name = "string",
    .name_len = 6,
    .print_default = ppp_c_flags_handler_string_print_default,
    .set_flag = ppp_c_flags_handler_string_set_flag,
};

#define PPP_C_FLAGS_STATE_NONE 0
#define PPP_C_FLAGS_STATE_FLAGS 1
#define PPP_C_FLAGS_STATE_FLAGS_SHORT 2

#define PPP_C_FLAGS_LOGER(c) (c | ('x' - 'X'))

static ppp_c_flags_malloc_f ppp_c_flags_malloc = malloc;
static ppp_c_flags_free_f ppp_c_flags_free = free;
static void ppp_c_flags_print_usage(ppp_c_flags_command_t *command);
const char *ppp_c_flags_error(int err)
{
    switch (err)
    {
    case PPP_C_FLAGS_ERROR_DUPLICATE_FLAG:
        return "duplicate flag name";
    case PPP_C_FLAGS_ERROR_DUPLICATE_FLAG_SHORT:
        return "duplicate flag short name";
    case PPP_C_FLAGS_ERROR_DUPLICATE_COMMAND:
        return "duplicate subcommand name";
    case PPP_C_FLAGS_ERROR_INVALID_FLAG:
        return "invalid flag name";
    case PPP_C_FLAGS_ERROR_INVALID_FLAG_SHORT:
        return "invalid flag short name";
    case PPP_C_FLAGS_ERROR_INVALID_FLAG_HELP:
        return "unable to use reserved flag --help";
    case PPP_C_FLAGS_ERROR_INVALID_FLAG_SHORT_HELP:
        return "unable to use reserved short flag -h";
    case PPP_C_FLAGS_ERROR_INVALID_FLAG_TYPE:
        return "invalid flag type";
    case PPP_C_FLAGS_ERROR_INVALID_COMMAND:
        return "invalid command name";
    case PPP_C_FLAGS_ERROR_MALLOC_FLAG:
        return "malloc flag fial";
    case PPP_C_FLAGS_ERROR_MALLOC_COMMAND:
        return "malloc command fial";
    case PPP_C_FLAGS_ERROR_MALLOC_ARGV:
        return "malloc argv fail";
    case PPP_C_FLAGS_ERROR_INVALID_ARGUMENT:
        return "invalid argument";
    case PPP_C_FLAGS_ERROR_UNKNOW_FLAG:
        return "unknow flag";
    case PPP_C_FLAGS_ERROR_UNKNOW_SHORT_FLAG:
        return "unknow short flag";
    }
    return "unknow error";
}
const char *ppp_c_flags_base_name(const char *filepath)
{
    size_t len = strlen(filepath);
    if (len > 2)
    {
        for (size_t i = len - 1; i > 0; i--)
        {
            switch (filepath[i])
            {
            case '/':
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
            case '\\':
#endif
                return filepath + i + 1;
            }
        }
    }
    return filepath;
}

// underscoreOK reports whether the underscores in s are allowed.
// Checking them in this one function lets all the parsers skip over them simply.
// Underscore must appear only between digits or between a base prefix and a digit.
uint8_t ppp_c_flags_underscore_ok(const char *s, size_t s_len)
{
    // saw tracks the last character (class) we saw:
    // ^ for beginning of number,
    // 0 for a digit or base prefix,
    // _ for an underscore,
    // ! for none of the above.
    char saw = '^';
    size_t i = 0;

    // Optional sign.
    if (s_len >= 1 && (s[0] == '-' || s[0] == '+'))
    {
        s++;
        s_len--;
    }

    // Optional base prefix.
    uint8_t hex = 0;
    if (s_len >= 2 && s[0] == '0')
    {
        switch (PPP_C_FLAGS_LOGER(s[1]))
        {
        case 'x':
            hex = 1;
        case 'b':
        case 'o':
            i = 2;
            saw = '0';
            break;
        }
    }

    // Number proper.
    for (; i < s_len; i++)
    {
        // Digits are always okay.
        if ('0' <= s[i] && s[i] <= '9' || hex && 'a' <= PPP_C_FLAGS_LOGER(s[i]) && PPP_C_FLAGS_LOGER(s[i]) <= 'f')
        {
            saw = '0';
            continue;
        }
        // Underscore must follow digit.
        if (s[i] == '_')
        {
            if (saw != '0')
            {
                return 0;
            }
            saw = '_';
            continue;
        }
        // Underscore must also be followed by digit.
        if (saw == '_')
        {
            return 0;
        }
        // Saw non-digit, non-underscore.
        saw = '!';
    }
    return saw != '_' ? 1 : 0;
}

int ppp_c_flags_parse_uint64(
    const char *s, size_t s_len,
    int base, int bit_size,
    uint64_t *output)
{
    if (!s || !s_len)
    {
        return -1;
    }
    uint8_t base0 = base == 0;
    const char *s0 = s;
    size_t s0_len = s_len;
    if (2 <= base && base <= 36)
    {
    }
    else if (!base)
    {
        base = 10;
        if (s[0] == '0')
        {
            if (s_len >= 3)
            {
                switch (PPP_C_FLAGS_LOGER(s[1]))
                {
                case 'b':
                    base = 2;
                    s += 2;
                    s_len -= 2;
                    break;
                case 'o':
                    base = 8;
                    s += 2;
                    s_len -= 2;
                    break;
                case 'x':
                    base = 16;
                    s += 2;
                    s_len -= 2;
                    break;
                default:
                    base = 8;
                    s++;
                    s_len--;
                    break;
                }
            }
            else
            {
                base = 8;
                s++;
                s_len--;
            }
        }
    }
    else
    {
        return -1;
    }
    if (!bit_size)
    {
        bit_size = 64;
    }
    else if (bit_size < 0 || bit_size > 64)
    {
        return -1;
    }
    // Cutoff is the smallest number such that cutoff*base > maxUint64.
    // Use compile-time constants for common cases.
    uint64_t cutoff = 18446744073709551615UL;
    switch (base)
    {
    case 10:
        cutoff = cutoff / 10 + 1;
        break;
    case 16:
        cutoff = cutoff / 16 + 1;
        break;
    default:
        cutoff = cutoff / base + 1;
        break;
    }
    uint64_t maxVal = 1;
    maxVal <<= bit_size;
    maxVal = -1;

    uint8_t underscores = 0;
    uint64_t n = 0, n1;
    uint8_t c, d;
    for (size_t i = 0; i < s_len; i++)
    {
        c = s[i];
        if (c == '_' && base0)
        {
            underscores = 1;
            continue;
        }
        else if ('0' <= c && c <= '9')
        {
            d = c - '0';
        }
        else
        {
            d = PPP_C_FLAGS_LOGER(c);
            if ('a' <= d && d <= 'z')
            {
                d -= 'a';
                d += 10;
            }
            else
            {
                return -1;
            }
        }
        if (d >= base)
        {
            return -1;
        }

        if (n >= cutoff)
        {
            // n*base overflows
            if (output)
            {
                *output = maxVal;
            }
            return 1;
        }
        n *= base;

        n1 = n + d;
        if (n1 < n || n1 > maxVal)
        {
            // n+d overflows
            if (output)
            {
                *output = maxVal;
            }
            return 1;
        }
        n = n1;
    }

    if (underscores && !ppp_c_flags_underscore_ok(s0, s0_len))
    {
        return -1;
    }
    if (output)
    {
        *output = n;
    }
    return 0;
}

int ppp_c_flags_parse_int64(
    const char *s, size_t s_len,
    int base, int bit_size,
    int64_t *output)
{
    if (!s || !s_len)
    {
        return -1;
    }
    // Pick off leading sign.
    const char *s0 = s;
    size_t s0_len = s_len;
    uint8_t neg = 0;
    switch (s[0])
    {
    case '-':
        neg = 1;
    case '+':
        s++;
        s_len--;
        break;
    }

    // Convert unsigned and check range.
    uint64_t un = 0;
    int err = ppp_c_flags_parse_uint64(s, s_len, base, bit_size, &un);
    if (err && err != 1)
    {
        return err;
    }
    if (bit_size == 0)
    {
        bit_size = 64;
    }
    uint64_t cutoff = 1;
    cutoff <<= (bit_size - 1);

    if (!neg && un >= cutoff)
    {
        if (output)
        {
            *output = cutoff - 1;
        }
        return 1;
    }
    if (neg && un > cutoff)
    {
        if (output)
        {
            *output = -(int64_t)(cutoff);
        }
        return 1;
    }
    if (output)
    {
        *output = un;
        if (neg)
        {
            *output = -*output;
        }
    }
    return 0;
}
int ppp_c_flags_parse_bool(const char *s, const size_t s_len)
{
    switch (s_len)
    {
    case 1:
        switch (s[0])
        {
        case '1':
        case 't':
        case 'T':
            return 1;
        case '0':
        case 'f':
        case 'F':
            return 0;
        }
        break;
    case 4:
        if (!memcmp(s, "true", 4) ||
            !memcmp(s, "TRUE", 4) ||
            !memcmp(s, "True", 4))
        {
            return 1;
        }
        break;
    case 5:
        if (!memcmp(s, "false", 5) ||
            !memcmp(s, "FALSE", 5) ||
            !memcmp(s, "False", 5))
        {
            return 0;
        }
        break;
    }
    return -1;
}
void ppp_c_flags_alloctor(ppp_c_flags_malloc_f m, ppp_c_flags_free_f f)
{
    ppp_c_flags_malloc = m ? m : malloc;
    ppp_c_flags_free = f ? f : free;
}
static uint8_t ppp_c_flags_check_flag(const char *s, const size_t s_len)
{
    if (!s || !s_len)
    {
        return 0;
    }
    for (size_t i = 0; i < s_len; i++)
    {
        switch (s[i])
        {
        case '"':
        case '\'':
        case '\r':
        case '\n':
            return 0;
        }
    }

    return 1;
}
static uint8_t ppp_c_flags_check_command(const char *s, const size_t s_len)
{
    if (!s || !s_len)
    {
        return 0;
    }
    if (s[0] == '-')
    {
        return 0;
    }
    for (size_t i = 0; i < s_len; i++)
    {
        switch (s[i])
        {
        case '"':
        case '\'':
        case '\r':
        case '\n':
            return 0;
        }
    }

    return 1;
}
ppp_c_flags_command_t *ppp_c_flags_command_create_with_len(
    const char *name, const size_t name_len,
    const char *describe,
    ppp_c_flags_command_handler_f handler, void *userdata,
    int *err)
{
    if (!ppp_c_flags_check_command(name, name_len))
    {
        if (err)
        {
            *err = PPP_C_FLAGS_ERROR_INVALID_COMMAND;
        }
        return 0;
    }
    ppp_c_flags_command_t *c = ppp_c_flags_malloc(sizeof(ppp_c_flags_command_t));
    if (!c)
    {
        if (err)
        {
            *err = PPP_C_FLAGS_ERROR_MALLOC_COMMAND;
        }
        return 0;
    }
    c->_parent = 0;
    c->_next = 0;
    c->_child = 0;
    c->_flag = 0;

    c->_name = name;
    c->_name_len = name_len;
    c->_describe = describe;

    c->handler = handler;
    c->userdata = userdata;
    if (err)
    {
        *err = 0;
    }
    return c;
}
ppp_c_flags_command_t *ppp_c_flags_command_create(
    const char *name,
    const char *describe,
    ppp_c_flags_command_handler_f handler, void *userdata,
    int *err)
{
    return ppp_c_flags_command_create_with_len(
        name, strlen(name),
        describe,
        handler, userdata,
        err);
}
void ppp_c_flags_command_destroy(ppp_c_flags_command_t *c)
{
    if (c->_next)
    {
        ppp_c_flags_command_destroy(c->_next);
    }
    if (c->_child)
    {

        ppp_c_flags_command_destroy(c->_child);
    }

    // destroy flags
    struct ppp_c_flags_flag *flag = c->_flag;
    struct ppp_c_flags_flag *flag_next;
    while (flag)
    {
        flag_next = flag->_next;
        ppp_c_flags_free(flag);
        flag = flag_next;
    }
    ppp_c_flags_free(c);
}
ppp_c_flags_command_t *ppp_c_flags_add_command_with_len(
    ppp_c_flags_command_t *parent,
    const char *name, size_t name_len,
    const char *describe,
    ppp_c_flags_command_handler_f handler, void *userdata,
    int *err)
{
    if (!ppp_c_flags_check_command(name, name_len))
    {
        if (err)
        {
            *err = PPP_C_FLAGS_ERROR_INVALID_COMMAND;
        }
        return 0;
    }
    ppp_c_flags_command_t *back = parent->_child;
    if (back)
    {
        while (back->_next)
        {
            if (name_len == back->_name_len && !memcmp(name, back->_name, name_len))
            {
                if (err)
                {
                    *err = PPP_C_FLAGS_ERROR_DUPLICATE_COMMAND;
                }
                return 0;
            }
            back = back->_next;
        }
        if (name_len == back->_name_len && !memcmp(name, back->_name, name_len))
        {
            if (err)
            {
                *err = PPP_C_FLAGS_ERROR_DUPLICATE_COMMAND;
            }
            return 0;
        }
    }

    ppp_c_flags_command_t *c = ppp_c_flags_malloc(sizeof(ppp_c_flags_command_t));
    if (!c)
    {
        if (err)
        {
            *err = PPP_C_FLAGS_ERROR_MALLOC_COMMAND;
        }
        return 0;
    }
    c->_parent = parent;
    c->_next = 0;
    c->_child = 0;
    c->_flag = 0;

    c->_name = name;
    c->_name_len = name_len;
    c->_describe = describe;

    c->handler = handler;
    c->userdata = userdata;

    if (back)
    {
        back->_next = c;
    }
    else
    {
        parent->_child = c;
    }
    if (err)
    {
        *err = 0;
    }
    return c;
}

ppp_c_flags_command_t *ppp_c_flags_add_command(
    ppp_c_flags_command_t *parent,
    const char *name,
    const char *describe,
    ppp_c_flags_command_handler_f handler, void *userdata,
    int *err)
{
    if (!name)
    {
        if (err)
        {
            *err = PPP_C_FLAGS_ERROR_INVALID_COMMAND;
        }
        return 0;
    }
    size_t name_len = strlen(name);
    if (!name_len)
    {
        if (err)
        {
            *err = PPP_C_FLAGS_ERROR_INVALID_COMMAND;
        }
        return 0;
    }
    return ppp_c_flags_add_command_with_len(
        parent,
        name, name_len,
        describe,
        handler, userdata,
        err);
}
ppp_c_flags_flag_t *ppp_c_flags_add_flag_with_len(
    ppp_c_flags_command_t *command,
    const char *name, const size_t name_len, char short_name,
    const char *describe,
    void *value, const int value_type,
    int *err)
{
    if (!ppp_c_flags_check_flag(name, name_len))
    {
        if (err)
        {
            *err = PPP_C_FLAGS_ERROR_INVALID_FLAG;
        }
        return 0;
    }
    else if (name_len == 4 && !memcmp(name, "help", 4))
    {
        if (err)
        {
            *err = PPP_C_FLAGS_ERROR_INVALID_FLAG_HELP;
        }
        return 0;
    }
    else if (short_name == 'h')
    {
        if (err)
        {
            *err = PPP_C_FLAGS_ERROR_INVALID_FLAG_SHORT_HELP;
        }
        return 0;
    }

    switch (value_type)
    {
    case PPP_C_FLAGS_TYPE_BOOL:
    case PPP_C_FLAGS_TYPE_INT64:
    case PPP_C_FLAGS_TYPE_UINT64:
    case PPP_C_FLAGS_TYPE_FLOAT64:
    case PPP_C_FLAGS_TYPE_STRING:
        break;
    default:
        if (err)
        {
            *err = PPP_C_FLAGS_ERROR_INVALID_FLAG_TYPE;
        }
        return 0;
    }
    struct ppp_c_flags_flag *back = command->_flag;
    if (back)
    {
        while (back->_next)
        {
            if (name_len == back->_name_len && !memcmp(name, back->_name, name_len))
            {
                if (err)
                {
                    *err = PPP_C_FLAGS_ERROR_DUPLICATE_FLAG;
                }
                return 0;
            }
            else if (short_name && short_name == back->_short_name)
            {
                if (err)
                {
                    *err = PPP_C_FLAGS_ERROR_DUPLICATE_FLAG_SHORT;
                }
                return 0;
            }
            back = back->_next;
        }
        if (name_len == back->_name_len && !memcmp(name, back->_name, name_len))
        {
            if (err)
            {
                *err = PPP_C_FLAGS_ERROR_DUPLICATE_FLAG;
            }
            return 0;
        }
        else if (short_name && short_name == back->_short_name)
        {
            if (err)
            {
                *err = PPP_C_FLAGS_ERROR_DUPLICATE_FLAG_SHORT;
            }
            return 0;
        }
    }

    struct ppp_c_flags_flag *flag = ppp_c_flags_malloc(sizeof(struct ppp_c_flags_flag));
    if (!flag)
    {
        if (err)
        {
            *err = PPP_C_FLAGS_ERROR_MALLOC_FLAG;
        }
        return 0;
    }
    memset(flag, 0, sizeof(struct ppp_c_flags_flag));
    // flag->_next = 0;

    void *p = &flag->_default;
    switch (value_type)
    {
    case PPP_C_FLAGS_TYPE_BOOL:
        *(PPP_C_FLAGS_BOOL *)p = *(PPP_C_FLAGS_BOOL *)value;
        flag->_handler = &ppp_c_flags_handler_bool;
        break;
    case PPP_C_FLAGS_TYPE_INT64:
        *(PPP_C_FLAGS_INT64 *)p = *(PPP_C_FLAGS_INT64 *)value;
        flag->_handler = &ppp_c_flags_handler_int64;
        break;
    case PPP_C_FLAGS_TYPE_UINT64:
        *(PPP_C_FLAGS_UINT64 *)p = *(PPP_C_FLAGS_UINT64 *)value;
        flag->_handler = &ppp_c_flags_handler_uint64;
        break;
    case PPP_C_FLAGS_TYPE_FLOAT64:
        *(PPP_C_FLAGS_FLOAT64 *)p = *(PPP_C_FLAGS_FLOAT64 *)value;
        flag->_handler = &ppp_c_flags_handler_float64;
        break;
    case PPP_C_FLAGS_TYPE_STRING:
        *(PPP_C_FLAGS_STRING *)p = *(PPP_C_FLAGS_STRING *)value;
        flag->_handler = &ppp_c_flags_handler_string;
        break;
    }

    flag->_name = name;
    flag->_name_len = name_len;
    flag->_short_name = short_name;
    flag->_describe = describe;

    flag->_value = value;
    flag->_type = value_type;
    if (back)
    {
        back->_next = flag;
    }
    else
    {
        command->_flag = flag;
    }
    if (err)
    {
        *err = 0;
    }
    return flag;
}
ppp_c_flags_flag_t *ppp_c_flags_add_flag(
    ppp_c_flags_command_t *command,
    const char *name, char short_name,
    const char *describe,
    void *value, const int value_type,
    int *err)
{
    if (!name)
    {
        if (err)
        {
            *err = PPP_C_FLAGS_ERROR_INVALID_FLAG;
        }
        return 0;
    }
    size_t name_len = strlen(name);
    if (!name_len)
    {
        if (err)
        {
            *err = PPP_C_FLAGS_ERROR_INVALID_FLAG;
        }
        return 0;
    }
    return ppp_c_flags_add_flag_with_len(
        command,
        name, name_len, short_name,
        describe,
        value, value_type,
        err);
}
typedef struct
{
    ppp_c_flags_command_t *command;
    int argc;
    char **argv;
    size_t i;
    char *s;
    size_t s_len;
    int *handler_result;

    int state;
    int err;

    int handler_argc;
    char **handler_argv;
    int handler_cap;

    uint8_t input : 1;
    uint8_t allow_unknow : 1;
    uint8_t short_input : 1;
} ppp_c_flags_execute_args_t;

static int ppp_c_flags_call(ppp_c_flags_execute_args_t *args)
{
    if (args->command->handler)
    {
        int ret = args->command->handler(
            args->command,
            args->handler_argc, args->handler_argv,
            args->command->userdata);
        if (args->handler_result)
        {
            *args->handler_result = ret;
        }
        if (args->handler_argv)
        {
            ppp_c_flags_free(args->handler_argv);
        }
    }
    return 1;
}
static int ppp_c_flags_push_args(ppp_c_flags_execute_args_t *args, char *s)
{
    if (args->handler_argc + 1 > args->handler_cap)
    {
        size_t cap = args->handler_argc ? args->handler_argc * 2 : 32;
        if (cap < 32)
        {
            cap = 32;
        }
        size_t max = args->argc - args->i;
        if (cap > max)
        {
            cap = max;
        }
        if (args->handler_argv)
        {
            void *p = ppp_c_flags_malloc(sizeof(const char *) * cap);
            if (!p)
            {
                ppp_c_flags_free(args->handler_argv);
                printf("malloc argv fail: %s\n", s);
                args->err = PPP_C_FLAGS_ERROR_MALLOC_ARGV;
                return 1;
            }
            if (args->handler_argc)
            {
                memcpy(p, args->handler_argv, args->handler_argc);
            }
            free(args->handler_argv);
            args->handler_argv = p;
        }
        else
        {
            args->handler_argv = ppp_c_flags_malloc(sizeof(const char *) * cap);
            if (!args->handler_argv)
            {
                printf("malloc argv fail: %s\n", s);
                args->err = PPP_C_FLAGS_ERROR_MALLOC_ARGV;
                return 1;
            }
            args->handler_cap = cap;
        }
    }
    args->handler_argv[args->handler_argc++] = s;
    args->i++;
    args->input = 1;
    args->state = PPP_C_FLAGS_STATE_NONE;
    return 0;
}

static void ppp_c_flags_print_name(ppp_c_flags_command_t *command)
{
    if (command->_parent)
    {
        ppp_c_flags_print_name(command->_parent);
        printf(" %s", command->_name);
    }
    else
    {
        printf("%s", command->_name);
    }
}
static uint8_t ppp_c_flags_is_delimiter(const char c)
{
    switch (c)
    {
    case 0:
    case '\n':
    case '\r':
        return 1;
    }
    return 0;
}

static void ppp_c_flags_print_flag(char *buf, size_t len, ppp_c_flags_flag_t *flag)
{
    if (flag)
    {
        printf("%s ", flag->_name);
        len -= flag->_name_len;
        sprintf(buf + 2, "%lds", len - 1);
        printf(buf, ((ppp_c_flags_flag_handler_t *)(flag->_handler))->name);
    }
    else
    {
        printf("help ");
        len -= 4;
        sprintf(buf + 2, "%lds", len - 1);
        printf(buf, "bool");
    }
}
static void ppp_c_flags_print_default(struct ppp_c_flags_flag *flag)
{
    if (flag->_describe && !ppp_c_flags_is_delimiter(flag->_describe[0]))
    {
        putchar(' ');
    }
    ((ppp_c_flags_flag_handler_t *)(flag->_handler))->print_default(flag);
}
static void ppp_c_flags_print_usage(ppp_c_flags_command_t *command)
{
    size_t i;
    char buf[2 + 21 + 1] = {0};
    buf[0] = '%';
    buf[1] = '-';
    size_t min;
    printf("Usage:\n  ");
    ppp_c_flags_print_name(command);
    if (command->_child)
    {
        printf(" [flags]\n  ");
        ppp_c_flags_print_name(command);
        printf(" [command]\n\nAvailable Commands:\n");
        ppp_c_flags_command_t *c = command->_child;
        min = 0;
        while (c)
        {
            if (min < c->_name_len)
            {
                min = c->_name_len;
            }
            c = c->_next;
        }
        if (min < 5)
        {
            min = 5;
        }

        sprintf(buf + 2, "%lds", min);
        c = command->_child;
        while (c)
        {
            printf("  ");
            printf(buf, c->_name);
            if (c->_describe && !ppp_c_flags_is_delimiter(c->_describe[0]))
            {
                printf("   ");
                for (i = 0; !ppp_c_flags_is_delimiter(c->_describe[i]); i++)
                {
                    putchar(c->_describe[i]);
                }
                putchar('\n');
            }
            else
            {
                putchar('\n');
            }
            c = c->_next;
        }
        putchar('\n');
    }
    else
    {
        printf(" [flags]\n\n");
    }
    printf("Flags:\n");
    struct ppp_c_flags_flag *flag = command->_flag;

    min = 4 + 1 + 5;
    while (flag)
    {
        i = flag->_name_len + 1 + ((ppp_c_flags_flag_handler_t *)(flag->_handler))->name_len;
        if (min < i)
        {
            min = i;
        }
        flag = flag->_next;
    }
    if (min < 11)
    {
        min = 11;
    }
    flag = command->_flag;
    while (flag)
    {
        if (flag->_short_name)
        {
            printf("  -%c, --", flag->_short_name);
            ppp_c_flags_print_flag(buf, min, flag);
            if (flag->_describe && !ppp_c_flags_is_delimiter(flag->_describe[0]))
            {

                printf("   ");
                for (i = 0; !ppp_c_flags_is_delimiter(flag->_describe[i]); i++)
                {
                    putchar(flag->_describe[i]);
                }
            }
            ppp_c_flags_print_default(flag);
            putchar('\n');
        }
        else
        {
            printf("      --");
            ppp_c_flags_print_flag(buf, min, flag);
            if (flag->_describe && !ppp_c_flags_is_delimiter(flag->_describe[0]))
            {
                printf("   ");
                for (i = 0; !ppp_c_flags_is_delimiter(flag->_describe[i]); i++)
                {
                    putchar(flag->_describe[i]);
                }
            }
            ppp_c_flags_print_default(flag);
            putchar('\n');
        }
        flag = flag->_next;
    }
    printf("  -h, --");
    ppp_c_flags_print_flag(buf, min, 0);
    printf("   Help for %s\n", command->_name);

    if (command->_child)
    {
        printf("\nUse \"%s [command] --help\" for more information about a command.\n", command->_name);
    }
}
void ppp_c_flags_print(ppp_c_flags_command_t *command)
{
    if (command->_describe)
    {
        puts(command->_describe);
        putchar('\n');
    }
    ppp_c_flags_print_usage(command);
}

static int ppp_c_flags_next_none(ppp_c_flags_execute_args_t *args)
{
    if (args->i >= args->argc)
    {
        return ppp_c_flags_call(args);
    }
    args->s = args->argv[args->i];
    args->s_len = strlen(args->s);
    // --
    if (args->s_len > 1)
    {
        if (args->s[0] == '-')
        {
            if (args->s[1] == '-')
            {
                if (args->s_len > 2)
                {
                    args->s += 2;
                    args->s_len -= 2;
                    args->input = 1;
                    args->state = PPP_C_FLAGS_STATE_FLAGS;
                    return 0;
                }
            }
            else
            {
                args->s++;
                args->s_len--;
                args->input = 1;
                args->short_input = 0;
                args->state = PPP_C_FLAGS_STATE_FLAGS_SHORT;
                return 0;
            }
        }
    }
    // subcommand
    if (!args->input && args->command->_child)
    {
        ppp_c_flags_command_t *c = args->command->_child;
        while (c)
        {
            if (c->_name_len == args->s_len && !memcmp(args->s, c->_name, args->s_len))
            {
                args->command = c;
                args->i++;
                return 0;
            }
            c = c->_next;
        }
    }
    // argv
    return ppp_c_flags_push_args(args, args->s);
}
static int ppp_c_flags_next_flags_set_value(ppp_c_flags_execute_args_t *args, struct ppp_c_flags_flag *flag, uint8_t isshort)
{
    if (PPP_C_FLAGS_TYPE_BOOL == flag->_type)
    {
        *(PPP_C_FLAGS_BOOL *)flag->_value = 1;
        args->i++;
        args->state = PPP_C_FLAGS_STATE_NONE;
        return 0;
    }
    if (args->i + 1 >= args->argc)
    {
        if (isshort)
        {
            printf("Error: flag needs an argument: -%c\n", flag->_short_name);
        }
        else
        {
            printf("Error: flag needs an argument: --%s\n", flag->_name);
        }
        ppp_c_flags_print_usage(args->command);
        return 1;
    }
    args->s = args->argv[args->i + 1];
    args->s_len = strlen(args->s);
    if (((ppp_c_flags_flag_handler_t *)(flag->_handler))->set_flag(flag, args->s, args->s_len))
    {
        if (isshort)
        {
            printf("Error: invalid argument for -%c: %s\n", flag->_short_name, args->s);
        }
        else
        {
            printf("Error: invalid argument for --%s: %s\n", flag->_name, args->s);
        }
        ppp_c_flags_print_usage(args->command);
        return 1;
    }
    args->i += 2;
    args->state = PPP_C_FLAGS_STATE_NONE;
    return 0;
}
static int ppp_c_flags_next_flags_set(ppp_c_flags_execute_args_t *args, struct ppp_c_flags_flag *flag, uint8_t isshort)
{
    switch (isshort)
    {
    case 0:
        args->s += (flag->_name_len + 1);
        args->s_len -= (flag->_name_len + 1);
        break;
    case 1:
        args->s += 2;
        args->s_len -= 2;
        break;
        // case 2:
    default:
        args->s++;
        args->s_len--;
        break;
    }
    if (((ppp_c_flags_flag_handler_t *)(flag->_handler))->set_flag(flag, args->s, args->s_len))
    {
        if (isshort)
        {
            printf("Error: invalid argument for -%c: %s\n", flag->_short_name, args->s);
        }
        else
        {
            printf("Error: invalid argument for --%s: %s\n", flag->_name, args->s);
        }
        ppp_c_flags_print_usage(args->command);
        return 1;
    }
    args->i++;
    args->state = PPP_C_FLAGS_STATE_NONE;
    return 0;
}
static int ppp_c_flags_next_flags(ppp_c_flags_execute_args_t *args)
{
    // --help
    if (args->s_len == 4 && !memcmp(args->s, "help", 4))
    {
        ppp_c_flags_print(args->command);
        return 1;
    }
    else if (args->s_len > 4 && !memcmp(args->s, "help=", 5))
    {
        args->s += 5;
        args->s_len -= 5;
        switch (ppp_c_flags_parse_bool(args->s, args->s_len))
        {
        case 0:
            args->i++;
            args->state = PPP_C_FLAGS_STATE_NONE;
            return 0;
        case 1:
            ppp_c_flags_print(args->command);
            return 1;
        default:
            printf("Error: invalid argument for --help: %s\n", args->s);
            ppp_c_flags_print_usage(args->command);
            args->err = PPP_C_FLAGS_ERROR_INVALID_ARGUMENT;
            return 1;
        }
    }
    else
    {
        struct ppp_c_flags_flag *flag = args->command->_flag;
        while (flag)
        {
            if (args->s_len == flag->_name_len &&
                !memcmp(args->s, flag->_name, flag->_name_len))
            {
                return ppp_c_flags_next_flags_set_value(args, flag, 0);
            }
            else if (args->s_len > flag->_name_len &&
                     args->s[flag->_name_len] == '=' &&
                     !memcmp(args->s, flag->_name, flag->_name_len))
            {
                return ppp_c_flags_next_flags_set(args, flag, 0);
            }
            flag = flag->_next;
        }
    }
    if (args->allow_unknow)
    {
        // argv
        return ppp_c_flags_push_args(args, args->s - 2);
    }
    printf("Error: unknown flag: --%s\n", args->s);
    ppp_c_flags_print_usage(args->command);
    args->err = PPP_C_FLAGS_ERROR_UNKNOW_FLAG;
    return 1;
}
static int ppp_c_flags_next_short(ppp_c_flags_execute_args_t *args)
{
    // -h
    if (args->s_len == 1 && args->s[0] == 'h')
    {
        ppp_c_flags_print(args->command);
        return 1;
    }
    else if (args->s_len > 1 && !memcmp(args->s, "h=", 2))
    {
        args->s += 2;
        args->s_len -= 2;
        switch (ppp_c_flags_parse_bool(args->s, args->s_len))
        {
        case 0:
            args->i++;
            args->state = PPP_C_FLAGS_STATE_NONE;
            return 0;
        case 1:
            ppp_c_flags_print(args->command);
            return 1;
        default:
            printf("Error: invalid argument for -h: %s\n", args->s);
            ppp_c_flags_print_usage(args->command);
            args->err = PPP_C_FLAGS_ERROR_INVALID_ARGUMENT;
            return 1;
        }
    }
    else
    {
        struct ppp_c_flags_flag *flag = args->command->_flag;
        while (flag)
        {
            if (flag->_short_name)
            {
                if (args->s_len == 1 &&
                    args->s[0] == flag->_short_name)
                {
                    return ppp_c_flags_next_flags_set_value(args, flag, 1);
                }
                else if (args->s_len > 1 &&
                         args->s[0] == flag->_short_name)
                {
                    if (args->s[1] == '=')
                    {
                        return ppp_c_flags_next_flags_set(args, flag, 1);
                    }
                    else if (flag->_type == PPP_C_FLAGS_TYPE_BOOL)
                    {
                        args->s++;
                        args->s_len--;
                        *(PPP_C_FLAGS_BOOL *)(flag->_value) = 1;
                        args->short_input = 1;
                        return 0;
                    }
                    else
                    {
                        return ppp_c_flags_next_flags_set(args, flag, 2);
                    }
                }
            }
            flag = flag->_next;
        }
    }
    if (!args->short_input && args->allow_unknow)
    {
        // argv
        return ppp_c_flags_push_args(args, args->s - 1);
    }
    printf("Error: unknown short flag: -%c\n", args->s[0]);
    ppp_c_flags_print_usage(args->command);
    args->err = PPP_C_FLAGS_ERROR_UNKNOW_SHORT_FLAG;
    return 1;
}

int ppp_c_flags_execute(
    ppp_c_flags_command_t *root,
    int argc, char **argv,
    uint8_t allow_unknow, int *handler_result)
{
    ppp_c_flags_execute_args_t args = {0};
    args.command = root;
    args.argc = argc;
    args.argv = argv;
    args.handler_result = handler_result;
    args.allow_unknow = allow_unknow;
    if (handler_result)
    {
        *handler_result = 0;
    }
    while (1)
    {
        switch (args.state)
        {
        case PPP_C_FLAGS_STATE_NONE:
            if (ppp_c_flags_next_none(&args))
            {
                return args.err;
            }
            break;
        case PPP_C_FLAGS_STATE_FLAGS:
            if (ppp_c_flags_next_flags(&args))
            {
                return args.err;
            }
            break;
        case PPP_C_FLAGS_STATE_FLAGS_SHORT:
            if (ppp_c_flags_next_short(&args))
            {
                return args.err;
            }
            break;
        }
    }
    return args.err;
}