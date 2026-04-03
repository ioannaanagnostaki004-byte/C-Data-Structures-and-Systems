#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_INPUT 1024  
#define MAX_ARGS 64     


void parse_command(char *input, char **args) {
    int i = 0;
    args[i] = strtok(input, " \t\n"); 
    while (args[i] != NULL) {
        i++;
        args[i] = strtok(NULL, " \t\n");
    }
}


void handle_redirection(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
           
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (fd < 0) {
                perror("open");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);  
            close(fd);
            args[i] = NULL;  
            break;
        } else if (strcmp(args[i], ">>") == 0) {
            
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (fd < 0) {
                perror("open");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);  
            close(fd);
            args[i] = NULL;  
            break;
        } else if (strcmp(args[i], "<") == 0) {
           
            int fd = open(args[i + 1], O_RDONLY);
            if (fd < 0) {
                perror("open");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);  
            close(fd);
            args[i] = NULL; 
            break;
        }
    }
}


int handle_pipe(char **args) {
    int pipe_fd[2];
    char *left[MAX_ARGS], *right[MAX_ARGS];
    int i = 0, j = 0, k = 0;

    while (args[i] != NULL && strcmp(args[i], "|") != 0) {
        left[j++] = args[i++];
    }
    left[j] = NULL;

    if (args[i] == NULL) {
        return 0; 
    }

    i++;
    while (args[i] != NULL) {
        right[k++] = args[i++];
    }
    right[k] = NULL;

    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid1 = fork();
    if (pid1 == 0) {
        
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        execvp(left[0], left);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else if (pid1 < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        
        dup2(pipe_fd[0], STDIN_FILENO);
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        execvp(right[0], right);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else if (pid2 < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    close(pipe_fd[0]);
    close(pipe_fd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    return 1;
}


void execute_shell() {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];

    while (1) {
        printf("my_shell$ ");
        fflush(stdout);

        
        if (fgets(input, MAX_INPUT, stdin) == NULL) {
            printf("\n");
            break;  
        }

      
        parse_command(input, args);

        if (args[0] == NULL) {
            continue;  
        }

        if (strcmp(args[0], "exit") == 0) {
            break;  
        }

       
        if (handle_pipe(args)) {
            continue;
        }

       
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            
            handle_redirection(args); 
            if (execvp(args[0], args) < 0) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        } else {
            
            wait(NULL);  
        }
    }
}

int main() {
    execute_shell(); 
    return 0;
}



