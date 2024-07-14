#include "flags.h"
#include "string.h"
#include <stdio.h>

#define PPP_C_FLAGS_STATE_NONE 0
#define PPP_C_FLAGS_STATE_FLAGS 1
#define PPP_C_FLAGS_STATE_FLAGS_SHORT 2

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
int ppp_c_flags_add_flag_with_len(
    ppp_c_flags_command_t *command,
    const char *name, const size_t name_len, char short_name,
    const char *describe,
    void *value, const int value_type)
{
    if (!ppp_c_flags_check_flag(name, name_len))
    {
        return PPP_C_FLAGS_ERROR_INVALID_FLAG;
    }
    else if (name_len == 4 && !memcmp(name, "help", 4))
    {
        return PPP_C_FLAGS_ERROR_INVALID_FLAG_HELP;
    }
    else if (short_name == 'h')
    {
        return PPP_C_FLAGS_ERROR_INVALID_FLAG_SHORT_HELP;
    }

    switch (value_type)
    {
    case PPP_C_FLAGS_TYPE_INT:
    case PPP_C_FLAGS_TYPE_UINT:
    case PPP_C_FLAGS_TYPE_BOOL:
    case PPP_C_FLAGS_TYPE_STRING:
        break;
    default:
        return PPP_C_FLAGS_ERROR_INVALID_FLAG_TYPE;
    }
    struct ppp_c_flags_flag *back = command->_flag;
    if (back)
    {
        while (back->_next)
        {
            if (name_len == back->_name_len && !memcmp(name, back->_name, name_len))
            {
                return PPP_C_FLAGS_ERROR_DUPLICATE_FLAG;
            }
            else if (short_name && short_name == back->_short_name)
            {
                return PPP_C_FLAGS_ERROR_DUPLICATE_FLAG_SHORT;
            }
            back = back->_next;
        }
        if (name_len == back->_name_len && !memcmp(name, back->_name, name_len))
        {
            return PPP_C_FLAGS_ERROR_DUPLICATE_FLAG;
        }
        else if (short_name && short_name == back->_short_name)
        {
            return PPP_C_FLAGS_ERROR_DUPLICATE_FLAG_SHORT;
        }
    }

    struct ppp_c_flags_flag *flag = ppp_c_flags_malloc(sizeof(struct ppp_c_flags_flag));
    if (!flag)
    {
        return PPP_C_FLAGS_ERROR_MALLOC_FLAG;
    }
    flag->_next = 0;

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
    return 0;
}
int ppp_c_flags_add_flag(
    ppp_c_flags_command_t *command,
    const char *name, char short_name,
    const char *describe,
    void *value, const int value_type)
{
    if (!name)
    {
        return PPP_C_FLAGS_ERROR_INVALID_FLAG;
    }
    size_t name_len = strlen(name);
    if (!name_len)
    {
        return PPP_C_FLAGS_ERROR_INVALID_FLAG;
    }
    return ppp_c_flags_add_flag_with_len(
        command,
        name, name_len, short_name,
        describe,
        value, value_type);
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
static size_t ppp_c_flags_print_flag_len(struct ppp_c_flags_flag *flag)
{
    switch (flag->_type)
    {
    case PPP_C_FLAGS_TYPE_INT:
        return flag->_name_len + 1 + 5;
    case PPP_C_FLAGS_TYPE_UINT:
        return flag->_name_len + 1 + 6;
    case PPP_C_FLAGS_TYPE_BOOL:
        return flag->_name_len + 1 + 4;
    // case PPP_C_FLAGS_TYPE_STRING:
    default:
        return flag->_name_len + 1 + 6;
    }
}
static void ppp_c_flags_print_flag(
    char *buf, size_t len,
    const char *name, size_t name_len,
    uint8_t value_type)
{
    printf("%s ", name);
    len -= name_len;
    sprintf(buf + 2, "%lds", len - 1);
    switch (value_type)
    {
    case PPP_C_FLAGS_TYPE_INT:
        printf(buf, "int64");
        break;
    case PPP_C_FLAGS_TYPE_UINT:
        printf(buf, "uint64");
        break;
    case PPP_C_FLAGS_TYPE_BOOL:
        printf(buf, "bool");
        break;
    // case PPP_C_FLAGS_TYPE_STRING:
    default:
        printf(buf, "string");
        break;
    }
}
static void ppp_c_flags_print_default(struct ppp_c_flags_flag *flag)
{
    if (!flag->_value)
    {
        return;
    }
    if (flag->_describe && !ppp_c_flags_is_delimiter(flag->_describe[0]))
    {
        putchar(' ');
    }
    switch (flag->_type)
    {
    case PPP_C_FLAGS_TYPE_INT:
    {
        int64_t val = *(int64_t *)flag->_value;
        if (val)
        {
            printf("<default: %ld>", val);
        }
    }
    break;
    case PPP_C_FLAGS_TYPE_UINT:
    {
        uint64_t val = *(uint64_t *)flag->_value;
        if (val)
        {
            printf("<default: %lu>", val);
        }
    }
    break;
    case PPP_C_FLAGS_TYPE_BOOL:
    {
        uint8_t val = *(uint8_t *)flag->_value;
        if (val)
        {
            printf("<default: true>");
        }
    }
    break;
    // case PPP_C_FLAGS_TYPE_STRING:
    default:
    {
        const char *val = *(char **)flag->_value;
        if (val && val[0] != 0)
        {
            printf("<default: %s>", val);
        }
    }
    break;
    }
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
        i = ppp_c_flags_print_flag_len(flag);
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
            ppp_c_flags_print_flag(
                buf, min,
                flag->_name, flag->_name_len,
                flag->_type);
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
            ppp_c_flags_print_flag(
                buf, min,
                flag->_name, flag->_name_len,
                flag->_type);
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
    ppp_c_flags_print_flag(buf, min, "help", 4, PPP_C_FLAGS_TYPE_BOOL);
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
        *(uint8_t *)flag->_value = 1;
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
    switch (flag->_type)
    {
    case PPP_C_FLAGS_TYPE_INT:
        if (ppp_c_flags_parse_int64(args->s, strlen(args->s), flag->_value))
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
        break;
    case PPP_C_FLAGS_TYPE_UINT:
        if (ppp_c_flags_parse_uint64(args->s, strlen(args->s), flag->_value))
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
        break;
    // case PPP_C_FLAGS_TYPE_STRING:
    default:
        *(uint8_t **)flag->_value = args->s;
        break;
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
    switch (flag->_type)
    {
    case PPP_C_FLAGS_TYPE_INT:
        if (ppp_c_flags_parse_int64(args->s, args->s_len, flag->_value))
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
        break;
    case PPP_C_FLAGS_TYPE_UINT:
        if (ppp_c_flags_parse_uint64(args->s, args->s_len, flag->_value))
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
        break;
    case PPP_C_FLAGS_TYPE_BOOL:
        switch (ppp_c_flags_parse_bool(args->s, args->s_len))
        {
        case 0:
            *(uint8_t *)flag->_value = 0;
            break;
        case 1:
            *(uint8_t *)flag->_value = 1;
            return 1;
        default:
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
        break;
    default:
        *(uint8_t **)flag->_value = args->s;
        break;
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
                     args->s[args->s_len] == '=' &&
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
                        args->i++;
                        args->state = PPP_C_FLAGS_STATE_NONE;
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
int ppp_c_flags_parse_uint64(const char *s, size_t s_len, uint64_t *output)
{
    return 0;
}
int ppp_c_flags_parse_int64(const char *s, size_t s_len, int64_t *output)
{
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