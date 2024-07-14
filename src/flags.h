#ifndef PPP_C_FLAGS__H
#define PPP_C_FLAGS__H
#include <stdint.h>
#include <stdlib.h>

#define PPP_C_FLAGS_VERSION 1

#define PPP_C_FLAGS_ERROR_DUPLICATE_FLAG 1
#define PPP_C_FLAGS_ERROR_DUPLICATE_FLAG_SHORT 2
#define PPP_C_FLAGS_ERROR_DUPLICATE_COMMAND 3
#define PPP_C_FLAGS_ERROR_INVALID_FLAG 4
#define PPP_C_FLAGS_ERROR_INVALID_FLAG_SHORT 5
#define PPP_C_FLAGS_ERROR_INVALID_FLAG_HELP 6
#define PPP_C_FLAGS_ERROR_INVALID_FLAG_SHORT_HELP 7
#define PPP_C_FLAGS_ERROR_INVALID_FLAG_TYPE 8
#define PPP_C_FLAGS_ERROR_INVALID_COMMAND 9
#define PPP_C_FLAGS_ERROR_MALLOC_FLAG 10
#define PPP_C_FLAGS_ERROR_MALLOC_COMMAND 11
#define PPP_C_FLAGS_ERROR_MALLOC_ARGV 12
/**
 * Get error string information
 */
const char *ppp_c_flags_error(int err);

#define PPP_C_FLAGS_TYPE_INT 1
#define PPP_C_FLAGS_TYPE_UINT 2
#define PPP_C_FLAGS_TYPE_BOOL 3
#define PPP_C_FLAGS_TYPE_STRING 4

#define PPP_C_FLAGS_INT int64_t
#define PPP_C_FLAGS_UINT uint64_t
#define PPP_C_FLAGS_BOOL uint8_t
#define PPP_C_FLAGS_STRING const char *

struct ppp_c_flags_flag
{
    // next flag
    struct ppp_c_flags_flag *_next;
    void *_value;
    uint8_t _type;

    const char *_name;
    size_t _name_len;
    char _short_name;
    const char *_describe;
};

typedef struct ppp_c_flags_command
{
    struct ppp_c_flags_command *_parent;
    struct ppp_c_flags_command *_next;
    struct ppp_c_flags_command *_child;
    struct ppp_c_flags_flag *_flag;

    const char *_name;
    size_t _name_len;
    const char *_describe;

    int (*handler)(struct ppp_c_flags_command *command, int argc, char **argv, void *userdata);
    void *userdata;
} ppp_c_flags_command_t;

typedef void *(*ppp_c_flags_malloc_f)(size_t n);
typedef void (*ppp_c_flags_free_f)(void *p);
typedef int (*ppp_c_flags_command_handler_f)(ppp_c_flags_command_t *command, int argc, char **argv, void *userdata);

/**
 * Returns the name of the filepath, usually using ppp_c_flags_base_name(argv[0]) to create the root command
 */
const char *ppp_c_flags_base_name(const char *filepath);
/**
 * Set how to allocate and release memory
 */
void ppp_c_flags_alloctor(ppp_c_flags_malloc_f m, ppp_c_flags_free_f f);

/**
 * Create a command
 */
ppp_c_flags_command_t *ppp_c_flags_command_create_with_len(
    const char *name, const size_t name_len,
    const char *describe,
    ppp_c_flags_command_handler_f handler, void *userdata,
    int *err);
/**
 * Create a command
 */
ppp_c_flags_command_t *ppp_c_flags_command_create(
    const char *name,
    const char *describe,
    ppp_c_flags_command_handler_f handler, void *userdata,
    int *err);
/**
 * Destroy all resources
 */
void ppp_c_flags_command_destroy(ppp_c_flags_command_t *c);

/**
 * Add subcommand
 */
ppp_c_flags_command_t *ppp_c_flags_add_command_with_len(
    ppp_c_flags_command_t *parent,
    const char *name, size_t name_len,
    const char *describe,
    ppp_c_flags_command_handler_f handler, void *userdata,
    int *err);

/**
 * Add subcommand
 */
ppp_c_flags_command_t *ppp_c_flags_add_command(
    ppp_c_flags_command_t *parent,
    const char *name,
    const char *describe,
    ppp_c_flags_command_handler_f handler, void *userdata,
    int *err);
/**
 * Add a flag
 */
int ppp_c_flags_add_flag_with_len(
    ppp_c_flags_command_t *command,
    const char *name, const size_t name_len, char short_name,
    const char *describe,
    void *value, const int value_type);
/**
 * Add a flag
 */
int ppp_c_flags_add_flag(
    ppp_c_flags_command_t *command,
    const char *name, char short_name,
    const char *describe,
    void *value, const int value_type);

/**
 * Instructions for using the output command
 */
void ppp_c_flags_print(ppp_c_flags_command_t *command);

/**
 * Parse parameters and execute command callback
 */
int ppp_c_flags_execute(
    ppp_c_flags_command_t *root,
    int argc, char **argv,
    int *handler_result);

#endif