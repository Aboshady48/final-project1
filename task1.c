#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

#define MAX_INPUT 1024
#define MAX_ARGS 100

// function to split input into args
void parse_input(char *input, char **args) {
    int i = 0;

    args[i] = strtok(input, " \n");

    while (args[i] != NULL) {
        i++;
        args[i] = strtok(NULL, " \n");
    }
}

int main() {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];

    while (1) {
        // 1. prompt
        printf("myShell> ");
        fflush(stdout);

        // 2. input
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
      // remove newline
      input[strcspn(input, "\n")] = '\0';

        // 3. parse
        parse_input(input, args);

        // ignore empty input
        if (args[0] == NULL) {
            continue;
        }
        
        // if user types "exit", terminate the shell loop
        if (strcmp(args[0], "exit") == 0) {
            break;
        }

	// 4. fork + exec
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
        }

        else if (pid == 0) {
            // child process
            if (execvp(args[0], args) == -1) {
                perror("command not found");
            }
            exit(1);
        }

        else {
            // parent process
            wait(NULL);
        }	        
       
    }

    return 0;
}
