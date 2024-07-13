#ifndef PPP_C_FLAGS__H
#define PPP_C_FLAGS__H
#include <stdint.h>
#include <stdlib.h>

#define PPP_C_FLAGS_ERROR_FLAGS_USED_REPEAT 1
#define PPP_C_FLAGS_ERROR_COMMAND_USED_REPEAT 2
#define PPP_C_FLAGS_ERROR_FLAGS_REPEAT 3
#define PPP_C_FLAGS_ERROR_FLAGS_SHORT_REPEAT 4
#define PPP_C_FLAGS_ERROR_COMMAND_REPEAT 5

#define PPP_C_FLAGS_TYPE_INT 1
#define PPP_C_FLAGS_TYPE_UINT 2
#define PPP_C_FLAGS_TYPE_BOOL 3
#define PPP_C_FLAGS_TYPE_STRING 4

typedef void *(*ppp_c_flags_malloc_f)(size_t n);
typedef void (*ppp_c_flags_free_f)(void *p);
typedef int (*ppp_c_flags_command_handler_f)(int argc, char **argv, void *userdata);

typedef struct ppp_c_flags_flags
{
    // next flag
    struct ppp_c_flags_flags *_next;
} ppp_c_flags_flags_t;

typedef struct ppp_c_flags_command
{
    struct ppp_c_flags_command *_next;
    struct ppp_c_flags_command *_child;
    struct ppp_c_flags_flags *_flags;

    const char *_name;
    size_t _name_len;
    const char *_describe;

    ppp_c_flags_command_handler_f handler;
    void *userdata;
} ppp_c_flags_command_t;

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
    ppp_c_flags_command_handler_f handler, void *userdata);
/**
 * Create a command
 */
ppp_c_flags_command_t *ppp_c_flags_command_create(
    const char *name,
    const char *describe,
    ppp_c_flags_command_handler_f handler, void *userdata);
/**
 * Destroy all resources
 */
void ppp_c_flags_command_destroy(ppp_c_flags_command_t *c);

/**
 * Add subcommand
 */
ppp_c_flags_command_t *ppp_c_flags_add_command(
    ppp_c_flags_command_t *parent,
    const char *name,
    const char *describe, void *userdata,
    int *err);
/**
 * Add flags
 */
ppp_c_flags_flags_t *ppp_c_flags_add_flags_with_len(
    ppp_c_flags_command_t *parent,
    const char *name, const size_t name_len, char short_name,
    const char *describe, void *value,
    int *err);
/**
 * Add flags
 */
ppp_c_flags_flags_t *ppp_c_flags_add_flags(
    ppp_c_flags_command_t *parent,
    const char *name, char short_name,
    const char *describe, void *value,
    int *err);

/**
 * Parse parameters and execute command callback
 */
int ppp_c_flags_execute(ppp_c_flags_command_t *parent, int argc, char **argv);

#endif