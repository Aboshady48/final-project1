#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/* ── Constants ─────────────────────────────────────────────────────────── */
#define MAX_INPUT    1024
#define MAX_ARGS      100
#define MAX_HISTORY   100

/* ── History storage ────────────────────────────────────────────────────── */
static char history[MAX_HISTORY][MAX_INPUT];
static int  history_count = 0;

static void parse_input(char *input, char **args) {
    int i = 0;
    args[i] = strtok(input, " \t\n");
    while (args[i] != NULL && i < MAX_ARGS - 1) {
        i++;
        args[i] = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
}

static void add_history(const char *line) {
    if (history_count < MAX_HISTORY) {
        strncpy(history[history_count], line, MAX_INPUT - 1);
        history[history_count][MAX_INPUT - 1] = '\0';
        history_count++;
    }
    /* If you want a rolling buffer instead of capping at 100, replace the
       above with a modulo index:  history[history_count % MAX_HISTORY]  */
}


static int handle_redirection(char **args) {
    for (int i = 0; args[i] != NULL; i++) {

        int is_out = (strcmp(args[i], ">")  == 0);
        int is_in  = (strcmp(args[i], "<")  == 0);

        if (!is_out && !is_in) continue;

        if (args[i + 1] == NULL) {
            fprintf(stderr, "myShell: syntax error: expected filename after '%s'\n",
                    args[i]);
            return -1;   /* caller should exit(1) */
        }

        int fd;
        if (is_out) {
            fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) { perror("myShell: open (output)"); exit(1); }
            if (dup2(fd, STDOUT_FILENO) < 0) { perror("myShell: dup2"); exit(1); }
        } else {
            fd = open(args[i + 1], O_RDONLY);
            if (fd < 0) { perror("myShell: open (input)");  exit(1); }
            if (dup2(fd, STDIN_FILENO)  < 0) { perror("myShell: dup2"); exit(1); }
        }
        close(fd);

        /* Shift args left by 2 to remove  operator + filename */
        int j = i;
        while (args[j + 2] != NULL) { args[j] = args[j + 2]; j++; }
        args[j]     = NULL;
        args[j + 1] = NULL;
        i--;   /* recheck same index after the shift */
    }
    return 0;
}

static void builtin_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "cd: missing argument\n");
    } else if (chdir(args[1]) != 0) {
        perror("cd");
    }
}

static void builtin_pwd(void) {
    char cwd[MAX_INPUT];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("pwd");
    }
}

static void builtin_history(void) {
    for (int i = 0; i < history_count; i++) {
        printf("%4d  %s\n", i + 1, history[i]);
    }
}

int main(void) {
    char  input[MAX_INPUT];
    char *args[MAX_ARGS];

    while (1) {
        /* ── 1. Prompt ── */
        printf("myShell> ");
        fflush(stdout);

        /* ── 2. Read input (Ctrl-D → EOF → clean exit) ── */
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");   /* tidy newline before exit */
            break;
        }

        /* Strip trailing newline */
        input[strcspn(input, "\n")] = '\0';

        /* Skip blank lines, but still record non-blank ones in history */
        if (input[0] == '\0') continue;
        add_history(input);   /* record BEFORE parse_input destroys the string */

        /* ── 3. Parse into tokens ── */
        parse_input(input, args);
        if (args[0] == NULL) continue;

        /* ── 4. Built-in commands (no fork needed) ── */
        if (strcmp(args[0], "exit") == 0)    break;
        if (strcmp(args[0], "cd")   == 0)  { builtin_cd(args);      continue; }
        if (strcmp(args[0], "pwd")  == 0)  { builtin_pwd();          continue; }
        if (strcmp(args[0], "history") == 0){ builtin_history();     continue; }

        /* ── 5. External command: fork → (redirect) → exec ── */
        pid_t pid = fork();

        if (pid < 0) {
            perror("myShell: fork");
            continue;
        }

        if (pid == 0) {
            /* ── Child process ── */
            if (handle_redirection(args) < 0) exit(1);  /* bad redirection syntax */
            execvp(args[0], args);
            /* execvp only returns on error */
            fprintf(stderr, "myShell: command not found: %s\n", args[0]);
            exit(1);
        }

        /* ── Parent process: wait for child ── */
        wait(NULL);
    }

    return 0;
}