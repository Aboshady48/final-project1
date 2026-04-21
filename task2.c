#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

#define MAX_INPUT 1024
#define MAX_ARGS 100
#define MAX_HISTORY 100

char history[MAX_HISTORY][MAX_INPUT];
int history_count = 0 ;
 
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
      // store commands in history array
      if(arg[0] != NULL){
        if (history_count < MAX_HISTORY) {
          strcpy(history[history_count], input);
          history_count++;
        }
    }

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
        // if user inputs cd <directory>
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL){ // user entered cd without directory
                fprintf(stderr,"Cd directory argument is missing \n")
            }else{
                if (chdir(args[1]) != 0) { // if fun returned 0 it successed 
                   perror("cd failed"); // entering if as it didnot return 0
                } 
            } 
            continue;
        }
        
        if (strcmp(args[0], "pwd") == 0) { // compare if zero cmd is pwd
            char cwd[1024]; // aray to save the current working directory 
            if (getcwd(cwd, sizeof(cwd)) != NULL) { // check if returns null pointer or not 
               printf("%s\n", cwd); // print the path
            } else {
                perror("pwd failed");
        }
            continue;
        }

        if (strcmp(args[0], "history") == 0) {
            //for loop on history array to show index of operation i+1 and input command 
            for(int i = 0 ; i < history_count ; i++){
                printf("%d %s\n", i+1 , history[i]);
            }
            continue;
        }
    // if not built in commands 
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
