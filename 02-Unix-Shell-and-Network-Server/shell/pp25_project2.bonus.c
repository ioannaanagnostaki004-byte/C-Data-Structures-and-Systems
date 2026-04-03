#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_INPUT 1024
#define MAX_ARGS 64
#define MAX_CMDS 16

void parse_command(char *input, char **args) {
    int i = 0;
    args[i] = strtok(input, " \t\n");
    while (args[i] != NULL) {
        i++;
        args[i] = strtok(NULL, " \t\n");
    }
}

int split_pipelines(char *input, char **commands) {
    int i = 0;
    commands[i] = strtok(input, "|");
    while (commands[i] != NULL) {
        i++;
        commands[i] = strtok(NULL, "|");
    }
    return i;
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

void handle_multiple_pipes(char **commands, int num_commands) {
    int pipe_fd[2 * (num_commands - 1)];
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipe_fd + i * 2) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i < num_commands; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            if (i > 0) {
                dup2(pipe_fd[(i - 1) * 2], STDIN_FILENO);
            }
            if (i < num_commands - 1) {
                dup2(pipe_fd[i * 2 + 1], STDOUT_FILENO);
            }
            for (int j = 0; j < 2 * (num_commands - 1); j++) {
                close(pipe_fd[j]);
            }
            char *args[MAX_ARGS];
            parse_command(commands[i], args);
            handle_redirection(args);
            execvp(args[0], args);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i < 2 * (num_commands - 1); i++) {
        close(pipe_fd[i]);
    }
    for (int i = 0; i < num_commands; i++) {
        wait(NULL);
    }
}

void execute_shell() {
    char input[MAX_INPUT];
    char *commands[MAX_CMDS];
    while (1) {
        printf("my_shell$ ");
        fflush(stdout);
        if (fgets(input, MAX_INPUT, stdin) == NULL) {
            printf("\n");
            break;
        }
        input[strcspn(input, "\n")] = 0;
        int num_commands = split_pipelines(input, commands);
        if (num_commands == 0) {
            continue;
        }
        if (strcmp(commands[0], "exit") == 0) {
            break;
        }
        if (num_commands > 1) {
            handle_multiple_pipes(commands, num_commands);
        } else {
            char *args[MAX_ARGS];
            parse_command(commands[0], args);
            pid_t pid = fork();
            if (pid == 0) {
                handle_redirection(args);
                if (execvp(args[0], args) < 0) {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            } else if (pid > 0) {
                wait(NULL);
            } else {
                perror("fork");
                exit(EXIT_FAILURE);
            }
        }
    }
}

int main() {
    execute_shell();
    return 0;
}


