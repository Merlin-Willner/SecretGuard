# SecretGuard

This project only parses input flags and prints what it understood.
It does not scan files yet.

## Build

You need a C compiler (gcc).

Targets:

  make        # builds the secretguard binary (default target)
  make run    # runs the application with path examples/demo_repo
  make clean  # removes build/ and secretguard binary

## Run

You must give a path OR use --stdin.

Examples:

  ./secretguard --max-depth 3 /path/to/scan
  ./secretguard --stdin
  ./secretguard --help
