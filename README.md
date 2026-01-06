# SecretGuard (Increment 1)

This project only parses input flags and prints what it understood.
It does not scan files yet.

## Build

You need a C compiler (gcc or clang).

With Makefile:

  make

Without Makefile:

  gcc -Iinclude -Wall -Wextra -O2 main.c cli.c -o secretguard

## Run

You must give a path OR use --stdin.

Examples:

  ./secretguard --max-depth 3 /path/to/scan
  ./secretguard --stdin
  ./secretguard --help
