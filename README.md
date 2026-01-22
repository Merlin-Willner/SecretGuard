# SecretGuard

## Build

You need a C compiler (gcc).

Targets:

- `make` & `make all`
  Builds the `secretguard` binary (default target).
- `make run`
  Runs the application with path `examples/demo_repo`.
- `make clean`
  Removes `build/` and the `secretguard` binary.

## Tests

Run the unit tests with:

- `make test`
  Builds and runs `build/test_all` (Unity tests in `tests/`).

## Run

You must give a path OR use --stdin.

Examples:

  ./secretguard --max-depth 3 path/to/scan
  ./secretguard --stdin
  ./secretguard --help

## Help

SecretGuard 0.1.0 (Linux/WSL)
Usage: ./secretguard [OPTIONS] <path>
Options:
  -h, --help         Show this help text
      --max-depth N  Limit how deep we recurse (default: -1 for unlimited)
                     Example: ./secretguard --max-depth 3 path/to/scan
      --threads N    Number of worker threads (default: 0 for auto)
                     Example: ./secretguard --threads 4 path/to/scan
      --stdin        Read from STDIN instead of a file path
                     Example:
                       ./secretguard --stdin <<'EOF'
                       paste the text to check for security risks
                       EOF
      --json         Output results as JSON
                     Example: ./secretguard --json path/to/scan
      --out FILE     Write results to FILE instead of stdout
                     Example: ./secretguard --out report.txt path/to/scan

Note: Provide a path (default: current directory) or use --stdin.

## Requirements (Section 4) - Implementation

1. Language/structure: C code split into multiple modules with headers and sources (`src/`, `include/`; e.g., `src/app.c`, `include/app.h`).
2. Limited Linux command: SecretGuard is a simplified `grep -R`/`find` for secrets; recursive walk and line scanning (`src/walk.c`, `src/scanner.c`).
3. Filesystem + argc/argv + Linux File API: argument parsing via `parse_arguments` (`src/cli.c`); file access via `open/read` (`src/scanner.c`); output to stdout or file (`src/app.c`).
4. Dynamic data structures: findings stored as a linked list (`src/scanner.c`); job queue in the thread pool (`src/thread_pool.c`).
5. stdin/stdout: `--stdin` uses `scanner_scan_stdin` (`src/scanner.c`); default output goes to stdout (`src/app.c`).
6. Threads for parallelism: parallel scan via `scanner_scan_parallel` + thread pool (`src/scanner_parallel.c`, `src/thread_pool.c`).
7. Synchronization: mutex/condition/semaphores protect queue and shutdown in the thread pool (`src/thread_pool.c`).
8. Build with gcc + Makefile targets: `Makefile` provides `all`, `clean`, `test`, `run` (gcc as compiler).
9. AI usage documented under "AI Usage".

## Git Hook

To enable the optional pre-commit hook:

  git config core.hooksPath .githooks

  chmod +x .githooks/pre-commit

The hook runs `secretguard` on staged files and blocks commits if findings are detected.

TestSecret: 

## AI Usage

We used Github Copilot, ChatGPT-5 as well as Codex models for the following tasks:
  - Initial project scaffolding as a consultant and pair programmer
  - Parsing command-line arguments
  - Generating and explaining regex patterns as well as the examples and demo_repos used in testing
  - Generating Makefile
  - Git hooks bash script
  - Creating Tests
  - pthreads
