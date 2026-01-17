# SecretGuard

## Build

You need a C compiler (gcc).

Targets:

- `make`
  Builds the `secretguard` binary (default target).
- `make run`
  Runs the application with path `examples/demo_repo`.
- `make clean`
  Removes `build/` and the `secretguard` binary.

## Run

You must give a path OR use --stdin.

Examples:

  ./secretguard --max-depth 3 /path/to/scan
  ./secretguard --stdin
  ./secretguard --help

## Help

SecretGuard 0.1.0
Usage: ./secretguard [OPTIONS] <path>
Options:
  -h, --help         Show this help text
      --max-depth N  Limit how deep we recurse (default: -1 for unlimited)
      --stdin        Read from STDIN instead of a file path
      --json         Output results as JSON

Note: Provide either a path or --stdin.

## Git Hook

To enable the optional pre-commit hook:

  git config core.hooksPath .githooks
  chmod +x .githooks/pre-commit

The hook runs `secretguard` on staged files and blocks commits if findings are detected.

## AI Usage

We used Github Copilot, ChatGPT-5 as well as Codex models for the following tasks:
  - Initial project scaffolding as a consultant and pair programmer
  - Parsing command-line arguments
  - Generating and explaining regex patterns as well as the examples and demo_repos used in testing
  - Generating Makefile
  - Git hooks bash script
  - pthreads 
