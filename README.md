# c-flags

This is a fully functional open source c command line parsing library. 

It is easy to use and has no dependencies, just copy src/flags.h and src/flags.c to your project.

It has complete functions and includes the following features:

* Support subcommands
* Supports long and short flags
* Flags support multiple data types and automatically convert command line strings to flags types.
* Supports array flag.
* Supports setting custom validators for flag.
* Automatically generate usage instructions for all commands and flags (-h, --help)

Index List:
  * [Background](#Background)
  * [Install](#Install)
  * [How To Use](#How-To-Use)
    * [Quick Start](#Quick-Start)
    * [Examples](#Examples)
    * [Create Command](#Create-Command)
    * [Add Flag](#Add-Flag)
    * [Flag Default](#Flag-Default)
    * [Auto Verify](#Auto-Verify)
    * [Print Float](#Print-Float)
# Background

I usually do not use C programming but use more convenient languages ​​​​such as golang. However, recently due to some reasons, I have become obsessed with some C projects, but I have not found a command line parsing library that I like. So I refer to some of my relevant experiences in golang and typescript, this command line parser was developed so that I can have a handy tool to write the main function while continuing to indulge in C.

# Install

This project uses MIT authorization, and the source code has no external dependencies, so you only need to
**src/flags.h src/flags.c** copied to your project

> Although c is very attractive, the lack of a unified and convenient module release mechanism is really a headache. Considering that different module release schemes are not compatible, for the sake of simplicity, this project is released directly as source code. Please add the source code directly to your project

# How To Use

# Quick Start

Usually a typedef struct should be defined to store the command line flags that need to be parsed:

```
// define a struct for command's flags
typedef struct
{
    PPP_C_FLAGS_BOOL debug;
    PPP_C_FLAGS_STRING addr;
    PPP_C_FLAGS_UINT64 port;
    PPP_C_FLAGS_INT64 protocol;
    PPP_C_FLAGS_FLOAT64 limit;
} root_flags_t;
```

> PPP_C_FLAGS_XXX macros define supported data types and it is recommended to use these macros


You also need to define a processing function to perform specific logic:
```
// define a handler for command, write the processing logic of the command here
static int root_handler(ppp_c_flags_command_t *command, int argc, char **argv, void *userdata);
```

* **userdata**: Usually it is the pointer of root_flags_t. You can directly access the parsed data through the pointer.
* **argc/argv**: This is all unrecognized parameters after parsing flags

After completing the above two steps, you need to inject them into the system. Just:
1. Call **ppp_c_flags_command_create** to create a command instance
2. Repeatedly call **ppp_c_flags_add_flag** to define the flags that need to be parsed
3. Call **ppp_c_flags_execute** to parse command line parameters and automatically call the command callback function
4. Call **ppp_c_flags_command_destroy** before main function exits to release memory

# Examples

* [src/exmaple/main.c](src/exmaple/main.c) a complete example
* [src/exmaple/value.c](src/exmaple/value.c) demonstrates how to define subcommands and all supported underlying data types
* [src/exmaple/value_array.c](src/exmaple/value_array.c) demonstrates how to define subcommands and all supported array types

# Create Command

Use ppp_c_flags_command_create to create the root command and ppp_c_flags_add_command to create a subcommand.

```
ppp_c_flags_command_t *ppp_c_flags_command_create(
    const char *name,
    const char *describe,
    ppp_c_flags_command_handler_f handler, void *userdata,
    int *err);

ppp_c_flags_command_t *ppp_c_flags_add_command(
    ppp_c_flags_command_t *parent,
    const char *name,
    const char *describe,
    ppp_c_flags_command_handler_f handler, void *userdata,
    int *err);
```

* **name**: A command name is specified, which affects the description generated when -h, --help is used, and the match string when looking for matching subcommands
* **describe**: This affects the description generated when -h, --help is used, which should describe to the user what the command does, or demonstrate some usage examples.
* **handler**: After the system parses the parameters, if the user requested this command, it will call this function to process the logic of the user's request.
* **userdata** This is arbitrary data passed to the processing function, but it is recommended to define a struct containing all the flags that need to be parsed. Here, an instance of the struct is passed in
* **err** err is an optional parameter. If an error occurs, you can use it to obtain the cause of the error.

# Add Flag

Call ppp_c_flags_add_flag to add a flag that needs to be parsed to the command.

```
ppp_c_flags_flag_t *ppp_c_flags_add_flag(
    ppp_c_flags_command_t *command,
    const char *name, char short_name,
    const char *describe,
    void *value, const int value_type,
    int *err);
```

* **name**: long flag name, it must be set.
* **short_name**: short flag name. If it is 0, it means short name is not supported.
* **describe**: Used to describe the meaning of this flag to the user.
* **value/value_type**: value and value_type must correspond one to one. value_type specifies the data type of flag, and value must be a pointer of this type. The parser will perform pointer operations according to the specified type, convert the string input by the user and write it to this pointer, so if the type and pointer are inconsistent, illegal memory access may occur.
* **err**: err is an optional parameter. If an error occurs, you can use it to obtain the cause of the error.

> PPP_C_FLAGS_TYPE_XXX PPP_C_FLAGS_TYPE_XXX macros define supported types, The PPP_C_FLAGS_XXX macro with the same name indicates the corresponding memory model of this type.

# Flag Default

Set value to a non-empty value before calling ppp_c_flags_add_flag, then ppp_c_flags_add_flag will save this non-empty value as the default value of this flag and use it when the user does not specify this flag.

# Auto Verify

The flag entered by the user will have its type automatically verified. For example, for a similar flag like uint8, if you enter a value other than 0 to 255, the ppp_c_flags_execute function will directly return -1 after printing an error.

In addition, ppp_c_flags_flag_t has a verify attribute that can receive a verification function. If it is set, the parser will automatically call it to verify whether the flag is valid (return 0 means valid).

```
// Verify value, if return not 0 indicates that the value is illegal
int (*verify)(struct ppp_c_flags_flag *flag, uint8_t value_type, void *old_value, void *new_value);
```

> verify will use ppp_c_flags_flag_t to occupy one more pointer's memory size. If you don't need this function, you can define the macro PPP_C_FLAGS_DISABLE_VERIFY as 1

# Print Float

Usually programs can output default values ​​​​for various data types very well, but for floating point the default is to use `printf("%g")` for output. You can specify a print function for ppp_c_flags_flag_t to modify the precision of the output.

```
// Used to set how to print values ​​on the console
int (*print)(struct ppp_c_flags_flag *flag, uint8_t value_type, void *value);
```

> print will use ppp_c_flags_flag_t to occupy one more pointer's memory size. If you don't need this function, you can define the macro PPP_C_FLAGS_DISABLE_PRINT as 1