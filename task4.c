#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>      // needed for open(), O_WRONLY, O_RDONLY, O_CREAT, O_TRUNC

#define MAX_INPUT 1024
#define MAX_ARGS  100

void parse_input(char *input, char **args) {
    int i = 0;
    args[i] = strtok(input, " \n\t");
    while (args[i] != NULL && i < MAX_ARGS - 1) {
        i++;
        args[i] = strtok(NULL, " \n\t");
    }
    args[i] = NULL;   // always NULL-terminate
}   
void handle_redirection(char **args) {
    for (int i = 0; args[i] != NULL; i++) {

        /* ── Output redirection:  cmd > file ── */
        if (strcmp(args[i], ">") == 0) {
            if (args[i + 1] == NULL) {
                fprintf(stderr, "myShell: syntax error: expected filename after '>'\n");
                exit(1);
            }

            /* Open (or create) the output file; truncate if it already exists */
            int fd = open(args[i + 1],
                          O_WRONLY | O_CREAT | O_TRUNC,
                          0644);          /* permissions: rw-r--r-- */
            if (fd < 0) {
                perror("myShell: open (output)");
                exit(1);
            }

            /* Make stdout point to our file */
            if (dup2(fd, STDOUT_FILENO) < 0) {
                perror("myShell: dup2 (output)");
                exit(1);
            }
            close(fd);   /* fd no longer needed; stdout is now the file */

            /* Remove ">  filename" from args so execvp won't see them */
            int j = i;
            while (args[j + 2] != NULL) { args[j] = args[j + 2]; j++; }
            args[j]     = NULL;
            args[j + 1] = NULL;
            i--;          /* re-check this index; something new is here now */
        }

        /* ── Input redirection:  cmd < file ── */
        else if (strcmp(args[i], "<") == 0) {
            if (args[i + 1] == NULL) {
                fprintf(stderr, "myShell: syntax error: expected filename after '<'\n");
                exit(1);
            }

            int fd = open(args[i + 1], O_RDONLY);
            if (fd < 0) {
                perror("myShell: open (input)");
                exit(1);
            }

            if (dup2(fd, STDIN_FILENO) < 0) {
                perror("myShell: dup2 (input)");
                exit(1);
            }
            close(fd);

            /* Remove "<  filename" from args */
            int j = i;
            while (args[j + 2] != NULL) { args[j] = args[j + 2]; j++; }
            args[j]     = NULL;
            args[j + 1] = NULL;
            i--;
        }
    }
}

int main(void) {
    char  input[MAX_INPUT];
    char *args[MAX_ARGS];

    while (1) {
        printf("myShell> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) break;

        input[strcspn(input, "\n")] = '\0';
        if (strlen(input) == 0) continue;

        parse_input(input, args);
        if (args[0] == NULL) continue;

        if (strcmp(args[0], "exit") == 0) break;

        pid_t pid = fork();
        if (pid < 0) {
            perror("myShell: fork");
        } else if (pid == 0) {
            /* child: handle redirection THEN exec */
            handle_redirection(args);
            execvp(args[0], args);
            fprintf(stderr, "myShell: command not found: %s\n", args[0]);
            exit(1);
        } else {
            wait(NULL);
        }
    }
    return 0;
}