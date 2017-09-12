#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "queue.h"

Queue hist;

ssize_t getcommand(char **lineptr, size_t *n);
int parsecommand(char *line, char ***commands, size_t *num);
int executecommand(char **commands, int pipe);
int executepipe(char **commands, int n_pipe);
void history(char **argv, int fd);
void updatehistory(char *str);

int runshell() {
        char *line = NULL;
        size_t len = 0;
        while(1) {
                ssize_t nread = getcommand(&line, &len);
                if (nread == -1) { // error

                }
                size_t num = 10;
                char **commands = malloc(num * sizeof(char*));
                int pipe = parsecommand(line, &commands, &num);
                if (pipe > 0) { // error
                        updatehistory(line);
                        int r = executecommand(commands, pipe);
                        if (r > 0) {
                            break;
                        }
                }

        }
        free(line);
        return 0;
}

ssize_t getcommand(char **lineptr, size_t *n) {
        printf("$");
        return getline(lineptr, n, stdin);
}

int parsecommand(char *line, char ***commands, size_t *num) {
        int n = 0;
        int i = 0;
        char *p = line;
        while(*p == ' ') {
                ++p;
        }
        if (*p == '|') {
                // parse error
                return -1;
        }
        while(*p != '\n') {
                char *q = p;
                while(*q != ' ' && *q != '|' && *q != '\n') {
                        ++q;
                }
                (*commands)[i] = malloc(q - p + 1);
                if ((*commands)[i] == NULL) {
                    // malloc error
                    return -1;
                }
                strncpy((*commands)[i], p, q - p);
                (*commands)[i++][q - p] = '\0';
                if (i >= *num - 1) {
                        *num *= 2;
                        char **temp = realloc(*commands, *num * sizeof(char**));
                        if (temp == NULL) {
                                return -1;
                        }
                        *commands = temp;
                        memset(*commands + *num / 2, 0, *num * sizeof(char**) / 2);
                }
                while(*q == ' ') {
                        ++q;
                }
                p = q;
                if (*q == '|') {
                        ++n;
                        (*commands)[i++] = NULL;
                        if (i >= *num) {
                                *num *= 2;
                                char **temp = realloc(*commands, *num * sizeof(char**));
                                if (temp == NULL) {
                                        return -1;
                                }
                                *commands = temp;
                                memset(*commands + *num / 2, 0, *num * sizeof(char**) / 2);
                        }
                        p = q + 1;
                        while(*p == ' ') {
                                ++p;
                        }
                        if (*p == '\n') {
                                // pipeline without next command
                                // error
                                return -1;
                        }
                        else if (*p == '|') {
                                // error
                                return -1;
                        }
                }
                //else if (*q == '\n') {
                //    break;
                //}
        }
        if (i > 0) {
                ++n;
        }
        return n;
}

int executecommand(char **commands, int n_pipe) {
        if (n_pipe == 1) {
                if (strcmp(commands[0], "cd") == 0) {
                        if (commands[1] && chdir(commands[1]) < 0) {
                                fprintf(stderr, "chdir error: %s\n", strerror(errno));
                        }
                }
                else if (strcmp(commands[0], "exit") == 0) {
                        return 1;
                }
                else if (strcmp(commands[0], "history") == 0) {

                }
                else {
                        pid_t pid = fork();
                        if (pid < 0) { // fork failed
                                fprintf(stderr, "fork error: %s\n", strerror(errno));
                        }
                        else if (pid == 0) { // child process
                                if (execve(commands[0], commands, NULL) < 0) {
                                        fprintf(stderr, "execve error: %s\n", strerror(errno));
                                        // exit the child process
                                }
                        }
                        else { // parent process
                                int status;
                                if (waitpid(pid, &status, 0) < 0) {
                                        fprintf(stderr, "waitpid error: %s\n", strerror(errno));
                                }
                        }
                }
        }
        else {
                return executepipe(commands, n_pipe);
        }
        return 0;
}

int executepipe(char **commands, int n_pipe) {
        int cmd = 0;
        int idx = 0;
        int *pipefd = malloc((n_pipe - 1) * 2 * sizeof(int));
        if (pipefd == NULL) {
                return -1;
        }
        while (cmd < n_pipe) {
                if (cmd < n_pipe - 1) {
                        pipe(pipefd + cmd * 2);
                }
                if (strcmp(commands[idx], "cd") == 0) {
                        if (commands[idx + 1] && chdir(commands[idx + 1]) < 0) {
                                fprintf(stderr, "chdir error: %s\n", strerror(errno));
                        }
                        if (cmd > 0) {
                                close(pipefd[(cmd - 1) * 2]);
                        }
                }
                else if (strcmp(commands[idx], "exit") == 0) {
                        if (cmd > 0) {
                                close(pipefd[(cmd - 1) * 2]);
                        }
                        return 1;
                }
                else if (strcmp(commands[idx], "history") == 0) {

                }
                else {
                        pid_t pid = fork();
                        if (pid < 0) {
                                fprintf(stderr, "fork error: %s\n", strerror(errno));
                        }
                        else if (pid == 0) { // child process
                                if (cmd < n_pipe - 1) {
                                        dup2(pipefd[cmd * 2 + 1], 1);
                                        close(pipefd[cmd * 2]);
                                        close(pipefd[cmd * 2 + 1]);
                                }
                                if (cmd > 0) {
                                        dup2(pipefd[(cmd - 1) * 2], 0);
                                        close(pipefd[(cmd - 1) * 2]);
                                }
                                if (execve(commands[idx], commands + idx, NULL) < 0) {
                                        fprintf(stderr, "execve error: %s\n", strerror(errno));
                                        return 1;
                                }
                        }
                        else { // parent process
                                if (cmd < n_pipe - 1) {
                                        close(pipefd[cmd * 2 + 1]);
                                }
                                if (cmd > 0) {
                                        close(pipefd[(cmd - 1) * 2]);
                                }
                                int status;
                                if (waitpid(pid, &status, 0) < 0) {
                                        fprintf(stderr, "waitpid error: %s\n", strerror(errno));
                                }
                        }
                }
                while (commands[idx] != NULL) {
                        ++idx;
                }
                ++idx;
                ++cmd;
        }
        return 0;
}

void history(char **argv, int fd) {
        if (argv[1] == NULL) {
                int i, qsize = queuesize(&hist);
                FILE *fpout = fdopen(fd, "w");
                if (fpout == NULL) {
                        fprintf(stderr, "error: %s\n", strerror(errno));
                        return;
                }
                for (i = 0; i < qsize; ++i) {
                        fprintf(fpout, "%d %s\n", i, queryqueue(&hist, i));
                }
                if (fclose(fpout) < 0) {
                        fprintf(stderr, "error: %s\n", strerror(errno));
                        return;
                }
        }
        else if (strcmp(argv[1], "-c")) {
                clearqueue(&hist);
        }
        else {

        }
}

void updatehistory(char *str) {
        if (queuefull(&hist)) {
                dequeue(&hist);
        }
        enqueue(&hist, str);
}
