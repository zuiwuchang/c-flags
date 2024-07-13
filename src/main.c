#include "flags.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
typedef struct
{

} main_flags_t;
int roo_handler(int argc, char **argv, void *userdata)
{
    main_flags_t *flags = userdata;
}
int main(int argc, char **argv)
{
    main_flags_t main_falgs;
    ppp_c_flags_command_t *root = ppp_c_flags_command_create(
        "example",
        "c-flags example",
        roo_handler,
        &main_falgs);
    if (!root)
    {
        printf("ppp_c_flags_command_create fail: %s\n", strerror(errno));
        return -1;
    }

    int ok = 0;
    ppp_c_flags_command_destroy(root);
    return ok;
}
