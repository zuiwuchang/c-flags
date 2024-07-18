#include "value_array.h"
#include <stdio.h>
static int value_array_handler(ppp_c_flags_command_t *command, int argc, char **argv, void *userdata)
{
    printf("value_array_handler:\n");
    value_array_flags_t *flags = userdata;
    printf("  i8=[");
    if (flags->i8.len)
    {
    }
    puts("]");
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
    return 0;
}