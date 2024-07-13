#include "flags.h"
#include "string.h"

static ppp_c_flags_malloc_f ppp_c_flags_malloc = malloc;
static ppp_c_flags_free_f ppp_c_flags_free = free;
void ppp_c_flags_alloctor(ppp_c_flags_malloc_f m, ppp_c_flags_free_f f)
{
    ppp_c_flags_malloc = m ? m : malloc;
    ppp_c_flags_free = f ? f : free;
}
ppp_c_flags_command_t *ppp_c_flags_command_create_with_len(
    const char *name, const size_t name_len,
    const char *describe,
    ppp_c_flags_command_handler_f handler, void *userdata)
{
    ppp_c_flags_command_t *c = ppp_c_flags_malloc(sizeof(ppp_c_flags_command_t));
    if (!c)
    {
        return 0;
    }
    c->_next = 0;
    c->_child = 0;
    c->_flags = 0;

    c->_name = name;
    c->_name_len = name_len;
    c->_describe = describe;

    c->handler = handler;
    c->userdata = userdata;
    return c;
}
ppp_c_flags_command_t *ppp_c_flags_command_create(
    const char *name,
    const char *describe,
    ppp_c_flags_command_handler_f handler, void *userdata)
{
    return ppp_c_flags_command_create_with_len(
        name, strlen(name),
        describe,
        handler, userdata);
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
    ppp_c_flags_flags_t *flags = c->_flags;
    ppp_c_flags_flags_t *flags_next;
    while (flags)
    {
        flags_next = flags->_next;
        ppp_c_flags_free(flags);
        flags = flags_next;
    }
}