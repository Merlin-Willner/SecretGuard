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

## Tests

Run the unit tests with:

- `make test`
  Builds and runs `build/test_all` (Unity tests in `tests/`).

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

      -h, --help     Show this help text
      --max-depth N  Limit how deep we recurse (default: -1 for unlimited)
      --threads N    Number of worker threads (default: 0 for auto)
      --stdin        Read from STDIN instead of a file path
      --json         Output results as JSON
      --out FILE     Write results to FILE instead of stdout

Note: Provide either a path or --stdin.

## Anforderungen (Abschnitt 4) - Umsetzung

1. C, mehrere Dateien, Header: `src/` und `include/` (z.B. `src/app.c`, `include/app.h`).
2. Begrenzter Linux-Command: SecretGuard ist ein "grep -R / find"-aehnlicher Scanner fuer Secrets in Dateien (`src/walk.c`, `src/scanner.c`).
3. Dateisystem + argc/argv + Linux File API: CLI-Parsing in `src/cli.c`, Datei-IO via `open/read` in `src/scanner.c`, Ausgabe via stdout/Datei in `src/app.c`.
4. Dynamische Datenstrukturen: Linked List fuer Findings in `src/scanner.c`; Job-Queue im Thread-Pool in `src/thread_pool.c`.
5. stdin/stdout: `--stdin` nutzt `scanner_scan_stdin` in `src/scanner.c`, Ausgabe standardmaessig auf stdout in `src/app.c`.
6. Threads fuer Parallelitaet: Parallel-Scanner in `src/scanner_parallel.c` mit `src/thread_pool.c`.
7. Synchronisation: Mutex/Cond/Semaphoren im Thread-Pool (`src/thread_pool.c`).
8. Build mit gcc + Makefile Targets: `Makefile` enthaelt `all`, `clean`, `test`, `run`.
9. Git-Commits + AI-Nutzung: Commit-Historie im Repo (`git log`); AI-Nutzung dokumentiert im Abschnitt "AI Usage".

## Git Hook

To enable the optional pre-commit hook:

  git config core.hooksPath .githooks

  chmod +x .githooks/pre-commit

The hook runs `secretguard` on staged files and blocks commits if findings are detected.

TestSecret: Password = JamesBond007

## AI Usage

We used Github Copilot, ChatGPT-5 as well as Codex models for the following tasks:
  - Initial project scaffolding as a consultant and pair programmer
  - Parsing command-line arguments
  - Generating and explaining regex patterns as well as the examples and demo_repos used in testing
  - Generating Makefile
  - Git hooks bash script
  - Creating Tests
  - pthreads
