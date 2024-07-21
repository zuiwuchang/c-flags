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
  * [How To Use](#HowToUse)

# Background

I usually do not use C programming but use more convenient languages ​​​​such as golang. However, recently due to some reasons, I have become obsessed with some C projects, but I have not found a command line parsing library that I like. So I refer to some of my relevant experiences in golang and typescript, this command line parser was developed so that I can have a handy tool to write the main function while continuing to indulge in C.

# Install

This project uses MIT authorization, and the source code has no external dependencies, so you only need to
**src/flags.h src/flags.c** copied to your project

> Although c is very attractive, the lack of a unified and convenient module release mechanism is really a headache. Considering that different module release schemes are not compatible, for the sake of simplicity, this project is released directly as source code. Please add the source code directly to your project

# How To Use