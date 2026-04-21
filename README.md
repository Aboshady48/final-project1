# final-project1

# myShell

**Custom Unix Shell — Project Documentation & Test Report**  
Faculty of Computers & Data Science · Alexandria University  
Operating Systems · Spring 2026

---

## Table of Contents

1. [Project Overview](#1-project-overview)
2. [How to Compile & Run](#2-how-to-compile--run)
3. [Code Architecture](#3-code-architecture)
4. [Task 4 — I/O Redirection Tests (Standalone)](#4-task-4--io-redirection-tests-standalone)
5. [Integration Tests — Tasks 1, 2, and 4](#5-integration-tests--tasks-1-2-and-4)
6. [Team Contributions](#6-team-contributions)

---

## 1. Project Overview

`myShell` is a functional command-line shell written in C. It mimics the core behaviour of standard Unix shells (`bash` / `sh`) and is built exclusively using the POSIX APIs required by the project specification: `fork()`, `exec()`, `wait()`, and `waitpid()`.

**Features implemented:**

| Task | Description |
|------|-------------|
| Task 1 | Prompt, user input, argument parsing, fork/exec pipeline |
| Task 2 | Built-in commands: `cd`, `pwd`, `history`, `exit` |
| Task 4 | I/O Redirection: `>` (output) and `<` (input) |

---

## 2. How to Compile & Run

**Compile:**
```bash
gcc -Wall -Wextra -o myShell myShell.c
```

**Run:**
```bash
./myShell
```

**Exit** by typing `exit` or pressing `Ctrl+D`.

---

## 3. Code Architecture

### Function Overview

| Function | Responsibility |
|----------|----------------|
| `parse_input()` | Tokenises the raw input string into an `argv`-style array using `strtok()` |
| `add_history()` | Copies the raw command line into the history buffer **before** `parse_input()` destroys it |
| `handle_redirection()` | Scans `args` for `>` / `<` tokens, opens files, `dup2`s file descriptors, then removes the operator and filename from `args` so `execvp()` never sees them |
| `builtin_cd()` | Calls `chdir()`; prints error on failure. No fork needed |
| `builtin_pwd()` | Calls `getcwd()` and prints the current working directory |
| `builtin_history()` | Iterates the history array and prints each entry with its index |
| `main()` | REPL loop: prompt → `fgets` → `add_history` → `parse_input` → dispatch built-in or fork/exec |

### Key Design Decisions

- **`add_history()` is called before `parse_input()`** — because `strtok()` modifies the string in-place by inserting `\0` characters. Calling it after would save a corrupted, partially-tokenised line into history.

- **`handle_redirection()` runs inside the child process** — it is called after `fork()` and before `execvp()`. This means only the child's file descriptors are changed; the shell's own `stdin`/`stdout` remain completely untouched.

- **All helper functions are `static`** — they are file-local and cannot pollute other translation units.

- **`dup2()` return value is checked** — a failed `dup2` would silently misdirect I/O without this guard.

---

## 4. Task 4 — I/O Redirection Tests (Standalone)

These tests apply to the **Task 4 standalone version** of `myShell` (redirection only, no built-ins).

Compile and launch before running these tests:
```bash
gcc -Wall -Wextra -o myShell myShell.c && ./myShell
```

---

### T4-01 — Basic Output Redirection (`>`)

**Objective:** Verify that the stdout of a command is written to a file instead of the terminal.

**Steps:**
```
myShell> ls > files.txt
myShell> cat files.txt
```

**Expected:** No output appears on the terminal after `ls`. Running `cat files.txt` shows the directory listing inside the file.

| Check | Expected | Actual | Status |
|-------|----------|--------|--------|
| No terminal output from `ls` | Screen is blank after `ls` | Blank screen | ✅ PASS |
| `files.txt` created | File exists in directory | File exists | ✅ PASS |
| File contains listing | Directory contents shown | Correct listing | ✅ PASS |

---

### T4-02 — Output Redirection Overwrites Existing File

**Objective:** Verify that `>` truncates an existing file and does not append to it.

**Steps:**
```
myShell> echo first line > test.txt
myShell> echo second line > test.txt
myShell> cat test.txt
```

**Expected:**
```
second line
```
Only the second `echo` appears — the file was overwritten.

| Check | Expected | Actual | Status |
|-------|----------|--------|--------|
| File contains only new content | `second line` only | Correct | ✅ PASS |
| `first line` is gone | Not present in file | Not present | ✅ PASS |

---

### T4-03 — Basic Input Redirection (`<`)

**Objective:** Verify that a command reads its `stdin` from a file.

**Steps:**
```bash
# Outside myShell, create the file first:
echo 'hello from file' > greet.txt

# Inside myShell:
myShell> cat < greet.txt
```

**Expected:**
```
hello from file
```

| Check | Expected | Actual | Status |
|-------|----------|--------|--------|
| `cat` reads from file | `hello from file` printed | Correct | ✅ PASS |

---

### T4-04 — Combined Input and Output Redirection

**Objective:** Verify that both `<` and `>` work together in a single command.

**Steps:**
```bash
# Create source file first:
echo -e 'cherry\napple\nbanana' > fruits.txt

# Inside myShell:
myShell> sort < fruits.txt > sorted.txt
myShell> cat sorted.txt
```

**Expected:**
```
apple
banana
cherry
```

| Check | Expected | Actual | Status |
|-------|----------|--------|--------|
| `sorted.txt` created | File exists | Exists | ✅ PASS |
| Content is sorted | Alphabetical order | Correct order | ✅ PASS |

---

### T4-05 — Redirect Output with Arguments

**Objective:** Verify that a command with multiple arguments still redirects correctly.

**Steps:**
```
myShell> echo Hello World > hello.txt
myShell> cat hello.txt
```

**Expected:**
```
Hello World
```

| Check | Expected | Actual | Status |
|-------|----------|--------|--------|
| File contains message | `Hello World` | Correct | ✅ PASS |

---

### T4-06 — Input from Non-Existent File (Error Handling)

**Objective:** Verify that a helpful error is printed when the input file does not exist.

**Steps:**
```
myShell> cat < ghost.txt
```

**Expected:**
```
myShell: open (input): No such file or directory
```

| Check | Expected | Actual | Status |
|-------|----------|--------|--------|
| Error message printed | `perror` message displayed | Correct | ✅ PASS |
| Shell continues running | Prompt reappears | Prompt reappears | ✅ PASS |

---

### T4-07 — Missing Filename After Operator (Syntax Error)

**Objective:** Verify that a syntax error is printed when no filename follows `>` or `<`.

**Steps:**
```
myShell> ls >
myShell> cat <
```

**Expected:**
```
myShell: syntax error: expected filename after '>'
myShell: syntax error: expected filename after '<'
```

| Check | Expected | Actual | Status |
|-------|----------|--------|--------|
| Syntax error for `>` | Clear error message | Correct | ✅ PASS |
| Syntax error for `<` | Clear error message | Correct | ✅ PASS |
| Shell does not crash | Prompt reappears | Prompt reappears | ✅ PASS |

---

## 5. Integration Tests — Tasks 1, 2, and 4

These tests verify that all three implemented tasks work **correctly together** in the final `myShell` binary.

Compile and launch:
```bash
gcc -Wall -Wextra -o myShell myShell.c && ./myShell
```

---

### 5.1 Task 1 — Prompt, Parsing, and External Commands

#### Test I-01 — Prompt Is Displayed

Start the shell and confirm `myShell>` appears before any input.

| Input | Expected | Actual | Status |
|-------|----------|--------|--------|
| (shell starts) | `myShell>` printed | Prompt shown | ✅ PASS |

---

#### Test I-02 — Run External Command with Arguments

Verify `fork`/`exec` dispatches correctly for external programs.

| Input | Expected | Actual | Status |
|-------|----------|--------|--------|
| `ls -l` | Long directory listing | Correct output | ✅ PASS |
| `echo hello world` | `hello world` printed | Correct | ✅ PASS |

---

#### Test I-03 — Unknown Command Error

| Input | Expected | Actual | Status |
|-------|----------|--------|--------|
| `blahblah` | `command not found` error | Error printed | ✅ PASS |
| Shell keeps running | Prompt reappears | Reappears | ✅ PASS |

---

#### Test I-04 — Empty Input Ignored

| Input | Expected | Actual | Status |
|-------|----------|--------|--------|
| (press Enter only) | Prompt reappears, no crash | Correct | ✅ PASS |

---

#### Test I-05 — Ctrl+D Exits Gracefully

| Input | Expected | Actual | Status |
|-------|----------|--------|--------|
| `Ctrl+D` (EOF) | Shell exits cleanly | Clean exit | ✅ PASS |

---

### 5.2 Task 2 — Built-in Commands

#### Test I-06 — `cd` Changes Directory

```
myShell> cd /tmp
myShell> pwd
```

| Step | Expected | Actual | Status |
|------|----------|--------|--------|
| `cd /tmp` | No error | No error | ✅ PASS |
| `pwd` | `/tmp` printed | `/tmp` shown | ✅ PASS |

---

#### Test I-07 — `cd` With No Argument Prints Error

| Input | Expected | Actual | Status |
|-------|----------|--------|--------|
| `cd` | `cd: missing argument` on stderr | Correct | ✅ PASS |

---

#### Test I-08 — `cd` to Invalid Path

| Input | Expected | Actual | Status |
|-------|----------|--------|--------|
| `cd /fakepath` | `perror` message; cwd unchanged | Correct | ✅ PASS |

---

#### Test I-09 — `pwd` Prints Working Directory

| Input | Expected | Actual | Status |
|-------|----------|--------|--------|
| `pwd` | Absolute path of cwd | Correct path | ✅ PASS |

---

#### Test I-10 — `history` Records and Displays Commands

Run a sequence of commands then call `history`. Verify every prior command appears in order with correct indices.

```
myShell> echo a
myShell> echo b
myShell> history
```

| Step | Expected | Actual | Status |
|------|----------|--------|--------|
| `echo a` | Outputs `a` | Correct | ✅ PASS |
| `echo b` | Outputs `b` | Correct | ✅ PASS |
| `history` | `1  echo a`, `2  echo b`, `3  history` | Correct order | ✅ PASS |

---

#### Test I-11 — `history` Records Raw Line Before Parsing

Verify that the raw input (with spaces intact) appears in history, not a tokenised or corrupted version.

| Input | Expected in history | Actual | Status |
|-------|---------------------|--------|--------|
| `ls   -la` | `ls   -la` stored as typed | Raw form stored | ✅ PASS |

---

#### Test I-12 — `exit` Terminates the Shell

| Input | Expected | Actual | Status |
|-------|----------|--------|--------|
| `exit` | Shell exits, returns to system prompt | Clean exit | ✅ PASS |

---

### 5.3 Task 4 — Redirection Inside Full Shell

#### Test I-13 — Redirect After `cd`

Change directory then redirect output. Verify the file appears in the new directory.

```
myShell> cd /tmp
myShell> ls > out.txt
myShell> cat out.txt
```

| Step | Expected | Actual | Status |
|------|----------|--------|--------|
| `cd /tmp` | cwd is `/tmp` | Changed | ✅ PASS |
| `ls > out.txt` | `/tmp/out.txt` created | File created | ✅ PASS |
| `cat out.txt` | Listing of `/tmp` | Correct | ✅ PASS |

---

#### Test I-14 — Redirection Command Appears in History

Run a redirection command and confirm it is saved verbatim in history.

```
myShell> echo hi > hi.txt
myShell> history
```

| Step | Expected | Actual | Status |
|------|----------|--------|--------|
| `echo hi > hi.txt` | File created | Created | ✅ PASS |
| `history` | `echo hi > hi.txt` in list | Stored correctly | ✅ PASS |

---

#### Test I-15 — Shell stdin/stdout Unaffected by Child Redirection

After a redirection command, the shell itself must still read from the terminal and write to the terminal normally.

```
myShell> ls > out.txt
myShell> pwd
myShell> echo test
```

| Step | Expected | Actual | Status |
|------|----------|--------|--------|
| `ls > out.txt` | Output goes to file | Correct | ✅ PASS |
| `pwd` | Path printed to terminal | Terminal output OK | ✅ PASS |
| `echo test` | `test` on terminal | Terminal output OK | ✅ PASS |

---

#### Test I-16 — Combined `< >` with History Verification

```
myShell> echo -e 'z\na\nm' > w.txt
myShell> sort < w.txt > s.txt
myShell> cat s.txt
myShell> history
```

| Step | Expected | Actual | Status |
|------|----------|--------|--------|
| `echo ... > w.txt` | `w.txt` created | Created | ✅ PASS |
| `sort < w.txt > s.txt` | `s.txt` has sorted lines | Sorted correctly | ✅ PASS |
| `cat s.txt` | `a`, `m`, `z` in order | Correct | ✅ PASS |
| `history` | Both commands in history | Both recorded | ✅ PASS |

---

## 6. Team Contributions

| Student Name | Student ID | Contribution |
|--------------|------------|--------------|
| | | |
| | | |
| | | |
| | | |
| | | |
| | | |
| | | |
| | | |
| | | |
| | | |