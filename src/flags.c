#include "flags.h"
#include "string.h"
#include <stdio.h>

static ppp_c_flags_malloc_f ppp_c_flags_malloc = malloc;
static ppp_c_flags_free_f ppp_c_flags_free = free;

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
static uint8_t ppp_c_flags_check(const char *s, const size_t s_len)
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
    if (!ppp_c_flags_check(name, name_len))
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
    if (!ppp_c_flags_check(name, name_len))
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
    if (!name || !name_len)
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
    uint8_t input;

    int handler_argc;
    char **handler_argv;
    int handler_cap;
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
                printf("malloc argv fail: %s", s);
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
            if (args->handler_argv)
            {
                printf("malloc argv fail: %s", s);
                args->err = PPP_C_FLAGS_ERROR_MALLOC_ARGV;
                return 1;
            }
            args->handler_cap = cap;
        }
    }
    args->handler_argv[args->handler_argc++] = s;

    return 0;
}
static uint8_t ppp_c_flags_start_with(const char *s, size_t s_len, const char *sub, size_t sub_len)
{
    if (!sub_len)
    {
        return 1;
    }
    if (s_len < sub_len)
    {
        return 0;
    }
    return memcmp(s, sub, sub_len) ? 0 : 1;
}

#define PPP_C_FLAGS_STATE_NONE 0
#define PPP_C_FLAGS_STATE_FLAGS 1
#define PPP_C_FLAGS_STATE_FLAGS_SHORT 2
static int ppp_c_flags_next(ppp_c_flags_execute_args_t *args)
{
    switch (args->state)
    {
    case PPP_C_FLAGS_STATE_NONE:
        if (args->i >= args->argc)
        {
            return ppp_c_flags_call(args);
        }
        args->s = args->argv[args->i++];
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
                        args->state = PPP_C_FLAGS_STATE_FLAGS;
                        return 0;
                    }
                }
                else
                {
                    args->s++;
                    args->s_len--;
                    args->state = PPP_C_FLAGS_STATE_FLAGS_SHORT;
                    return 0;
                }
            }
        }
        // subcommand
        if (!args->input && args->command->_child)
        {
            ppp_c_flags_command_t *c = args->command->_child;
            if (c->_name_len == args->s_len && !memcmp(args->s, c->_name, args->s_len))
            {
                args->command = c;
                args->i++;
                return 0;
            }
        }
        // argv
        return ppp_c_flags_push_args(args, args->s);
    case PPP_C_FLAGS_STATE_FLAGS:
        // --help
        if (args->s_len == 4 && !memcmp(args->s, "help", 4))
        {
            ppp_c_flags_print(args->command);
            return 1;
        }
        break;
    case PPP_C_FLAGS_STATE_FLAGS_SHORT:
        break;
    }
    return 0;
}
void ppp_c_flags_print_name(ppp_c_flags_command_t *command)
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
uint8_t ppp_c_flags_is_delimiter(const char c)
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
void ppp_c_flags_print_usage(ppp_c_flags_command_t *command)
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
int ppp_c_flags_execute(
    ppp_c_flags_command_t *root,
    int argc, char **argv,
    int *handler_result)
{
    ppp_c_flags_execute_args_t args = {0};
    args.command = root;
    args.argc = argc;
    args.argv = argv;
    args.handler_result = handler_result;

    while (!ppp_c_flags_next(&args))
    {
    }
    return args.err;
}