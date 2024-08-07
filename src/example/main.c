#include "../flags.h"
#include "value.h"
#include "value_array.h"
#include <stdio.h>
#include <string.h>

// define a struct for command's flags
typedef struct
{
    PPP_C_FLAGS_BOOL debug;
    PPP_C_FLAGS_STRING addr;
    PPP_C_FLAGS_UINT64 port;
    PPP_C_FLAGS_INT64 protocol;
    PPP_C_FLAGS_FLOAT64 limit;
} root_flags_t;
// define a handler for command, write the processing logic of the command here
static int root_handler(ppp_c_flags_command_t *command, int argc, char **argv, void *userdata)
{
    // userdata is a user-defined pointer, but if there are no additional requirements, it is usually a pointer to flags.
    root_flags_t *flags = userdata;

    // Different logic is executed based on the flags passed in.
    // For the simplicity of demonstration, the flags are just printed here.
    printf(
        "root_handler:\n  debug=%s\n  addr=%s\n  port=%lu\n  protocol=%ld\n  limit=%g\n",
        flags->debug ? "true" : "false",
        flags->addr,
        flags->port,
        flags->protocol,
        flags->limit);

    // Other argv, contains unknown flags
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
int main(int argc, char **argv)
{
    int err;
    // Create root command
    root_flags_t main_falgs = {0};
    char describe[16 + (3 + 1) * 3] = {0};
    memcpy(describe, "c-flags example ", 16);
    const char *version = ppp_c_flags_version();
    memcpy(describe + 16, version, strlen(version));
    ppp_c_flags_command_t *root = ppp_c_flags_command_create(
        ppp_c_flags_base_name(argv[0]),
        describe,
        root_handler, &main_falgs,
        &err);
    if (!root)
    {
        printf("Create root command fail: %s\n", ppp_c_flags_error(err));
        return -1;
    }
    // Create flags for commands
    if (!ppp_c_flags_add_flag(
            root,
            "debug", 'd', // 0 not use short flags
            "Run in debug mode",
            &main_falgs.debug, PPP_C_FLAGS_TYPE_BOOL, // Bind flags to memory for the parser to store parsed values
            &err))
    {
        printf("Add flags fail: %s\n", ppp_c_flags_error(err));
        goto FAIL;
    }

    main_falgs.addr = "127.0.0.1"; // Set default value ​​for bind parameters
    if (!ppp_c_flags_add_flag(
            root,
            "addr", 'a', // 'a' as flags short name, "-a xxx" "-a=xxx" "-axxx"
            "TCP bind ip address",
            &main_falgs.addr, PPP_C_FLAGS_TYPE_STRING,
            &err))
    {
        printf("Add flags fail: %s\n", ppp_c_flags_error(err));
        goto FAIL;
    }

    main_falgs.port = 80;
    if (!ppp_c_flags_add_flag(
            root,
            "port", 'p',
            "TCP bind port",
            &main_falgs.port, PPP_C_FLAGS_TYPE_UINT64,
            &err))
    {
        printf("Add flags fail: %s\n", ppp_c_flags_error(err));
        goto FAIL;
    }

    main_falgs.protocol = -1;
    if (!ppp_c_flags_add_flag(
            root,
            "protocol", 'P',
            "Communication protocol version used",
            &main_falgs.protocol, PPP_C_FLAGS_TYPE_INT64,
            &err))
    {
        printf("Add flags fail: %s\n", ppp_c_flags_error(err));
        goto FAIL;
    }

    main_falgs.limit = 0.8;
    if (!ppp_c_flags_add_flag(
            root,
            "limit", 'l',
            "CPU maximum usage",
            &main_falgs.limit, PPP_C_FLAGS_TYPE_FLOAT64,
            &err))
    {
        printf("Add flags fail: %s\n", ppp_c_flags_error(err));
        goto FAIL;
    }
    value_flags_t value_flags = {0};
    if (init_value_command(root, &value_flags))
    {
        goto FAIL;
    }
    value_array_flags_t value_array_falgs = {0};
    if (init_value_array_command(root, &value_array_falgs))
    {
        goto FAIL;
    }

    // Parse and execute commands
    int handler_result = 0;
    err = ppp_c_flags_execute(
        root,
        argc - 1, argv + 1, // Remove program startup path
        1, &handler_result);
    if (handler_result && !err)
    {
        err = handler_result;
    }
FAIL:
    // Clean up resources used by commands
    ppp_c_flags_command_destroy(root);
    return err;
}
