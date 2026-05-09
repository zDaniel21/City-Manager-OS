# AI Usage Documentation — Phases 1 and 2

## Tool Used
Claude (claude-sonnet-4-6), accessed via claude.ai

---

# Phase 1

## Prompt 1 — parse_condition

**What I asked:**
> I have a C struct called Report with the following fields:
> - id (int)
> - inspector (char[64])
> - latitude (double)
> - longitude (double)
> - category (char[32])
> - severity (int)
> - timestamp (time_t)
> - description (char[256])
>
> Generate a function:
>   int parse_condition(const char *input, char *field, char *op, char *value);
> that splits a string of the form field:operator:value into its three parts.
> Operators can be ==, !=, <, <=, >, >=.

**What was generated:**
The AI generated a function using `strtok()` to split the input string on `:` delimiters.
It correctly identified that multi-character operators like `<=` and `>=` need to be
treated as strings, not single characters.

**What I changed:**
`strtok()` modifies the original string by inserting null bytes. Since the input comes
directly from `argv[]`, which should not be modified, I rewrote the function using
pointer arithmetic with `strchr()` instead. This finds the colon positions without
touching the original string.

---

## Prompt 2 — match_condition

**What I asked:**
> Using the same Report struct above, generate a function:
>   int match_condition(Report *r, const char *field, const char *op, const char *value);
> that returns 1 if the record satisfies the condition and 0 otherwise.
> Supported fields: severity, category, inspector, timestamp.
> Supported operators: ==, !=, <, <=, >, >=.

**What was generated:**
The AI generated a function that correctly split the logic into string fields
(category, inspector) and numeric fields (severity, timestamp). It used `strcmp()`
for string comparisons and `atoi()` for converting the value string to a number
for numeric comparisons.

**What I changed:**
The AI used `atoi()` for the timestamp field. Since `time_t` is typically a 64-bit
value on modern Linux systems, `atoi()` would silently truncate timestamps beyond
the 32-bit integer range. I changed this to `atol()` to correctly handle the full
range of Unix timestamps.

I also added `fprintf(stderr, ...)` warnings for unknown fields and unsupported
operators, since the AI left these cases silent — just returning 0 with no indication
of what went wrong, which would make debugging very difficult.

---

## What I wrote myself — Phase 1

The `filter_reports()` function was written entirely by me without AI assistance,
as required by the spec. It:
- Opens `reports.dat` using `open()`
- Reads records one by one using `read()`
- Calls `parse_condition()` upfront for each condition to catch format errors early
- Tests each record against every condition using `match_condition()`
- Prints records where all conditions return 1 (AND logic)

---

## What I learned — Phase 1

- `strtok()` is destructive — it modifies the original string by inserting null bytes
  at delimiter positions. For read-only inputs like `argv[]`, pointer arithmetic
  with `strchr()` is the correct approach.
- `time_t` is typedef'd to `long` on 64-bit Linux, so using `atoi()` (which returns
  int) would silently truncate timestamps after year 2038 on 32-bit systems.
- AI-generated code is mostly correct for well-defined problems but needs careful
  review for edge cases — in both functions the logic was right but the
  implementation details needed fixing.
- The exercise of reviewing the generated code line by line was more valuable than
  just using it directly — it forced understanding of every decision made.

---

# Phase 2

## AI Usage in Phase 2

In Phase 2, AI was not used to generate any functional code. All implementation
was written by me, including:
- `district_remove()` in `district.c` — fork/exec pattern with `waitpid()`
- `monitor_main.c` — signal handling with `sigaction()`, PID file management
- `monitor.c` — reading `.monitor_pid` and sending SIGUSR1 with `kill()`
- Modifications to `main.c` — `remove_district` command and monitor notification

## AI used for — understanding concepts

AI was consulted to clarify two concepts before writing the code myself:

**1. sigaction() vs signal()**
> Why does the spec say not to use signal() and to use sigaction() instead?

The AI explained that `signal()` has undefined behavior in POSIX when called
from within a signal handler, and its behavior varies across implementations.
`sigaction()` is the correct POSIX interface — it gives explicit control over
signal masks, restart behavior (`SA_RESTART`), and is safe to use across all
UNIX systems. I used this understanding to correctly set up both SIGUSR1 and
SIGINT handlers in `monitor_main.c`.

**2. pause() in a signal loop**
> What is the correct way to wait for signals without busy-waiting?

The AI explained that `pause()` suspends the process until any signal is
received, making it the correct and efficient way to implement a signal-driven
wait loop. I wrote the `while (running) pause()` loop myself based on this
understanding.

## What I wrote myself — Phase 2

Everything in Phase 2 was written by me. The AI was only consulted for
conceptual clarification, not code generation.

## What I learned — Phase 2

- `fork()` + `execl()` is the standard UNIX pattern for running external commands
  from within a C program. The parent must call `waitpid()` to avoid zombie processes.
- `sigaction()` gives much finer control over signal handling than `signal()` —
  particularly the `SA_RESTART` flag which automatically restarts interrupted
  system calls.
- `kill()` with a PID read from a file is a simple but effective IPC mechanism
  between two unrelated processes.
- Writing the PID to a file on startup and deleting it on exit is a standard
  UNIX daemon pattern used in real-world software.