#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include "queue.h"
#include "graph.h"

Queue g_Hist;
Graph g_Edge;
int g_Switch;

ssize_t getcommand(char **lineptr, size_t *n);
int parsecommand(char *line, char ***commands, size_t *num);
int executecommand(char **commands, int pipe);
int executepipe(char **commands, int n_pipe, int outfd);
void concurrentpipe(char **commands, int n_pipe, int outfd);
void history(char **argv, int fd);
int checkrecursion(char **commands, int n_pipe);
void updatehistory(char *str);
int str2number(char *str);
int logerror(char *str);

int runshell() {
        char *line = NULL;
        size_t len = 0;
        size_t num = 5;
        char **commands = malloc(num * sizeof(char*));
        if (commands == NULL) {
                logerror(strerror(errno));
                return 1;
        }
        memset(commands, 0, num * sizeof(char*));
        initqueue(&g_Hist);
        initgraph(&g_Edge);
        g_Switch = 1;
        while(g_Switch) {
                ssize_t n_read;
                int n_pipe, i;
                n_read = getcommand(&line, &len);
                if (n_read < 0) { // error
                        logerror(strerror(errno));
                        fflush(stdin);
                        continue;
                }
                n_pipe = parsecommand(line, &commands, &num);
                if (n_pipe > 0) {
                        updatehistory(line);
                        //executepipe(commands, n_pipe, 1);
                        if (checkrecursion(commands, n_pipe)) {
                                logerror("infinite recursion");
                        }
                        else {
                                concurrentpipe(commands, n_pipe, 1);
                        }
                }
                for (i = 0; i < num; ++i) {
                        free(commands[i]);
                        commands[i] = NULL;
                }

        }
        free(line);
        free(commands);
        clearqueue(&g_Hist);
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
        char *syntaxerr = "syntax error near unexpected token `|`";
        while(*p == ' ') {
                ++p;
        }
        if (*p == '|') {
                // parse error
                logerror(syntaxerr);
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
                    logerror(strerror(errno));
                    return -1;
                }
                strncpy((*commands)[i], p, q - p);
                (*commands)[i++][q - p] = '\0';
                if (i >= *num - 1) {
                        char **temp;
                        temp = realloc(*commands, *num * 2 * sizeof(char**));
                        if (temp == NULL) {
                                //realloc error
                                logerror(strerror(errno));
                                return -1;
                        }
                        memset(temp + *num, 0, *num * sizeof(char**));
                        *commands = temp;
                        *num *= 2;
                }
                while(*q == ' ') {
                        ++q;
                }
                p = q;
                if (*q == '|') {
                        ++n;
                        (*commands)[i++] = NULL;
                        if (i >= *num - 1) {
                                char **temp;
                                temp = realloc(*commands, *num * 2 * sizeof(char**));
                                if (temp == NULL) {
                                        return -1;
                                }
                                memset(temp + *num, 0, *num * sizeof(char**));
                                *commands = temp;
                                *num *= 2;
                        }
                        p = q + 1;
                        while(*p == ' ') {
                                ++p;
                        }
                        if (*p == '\n') {
                                // pipeline without next command
                                // error
                                logerror(syntaxerr);
                                return -1;
                        }
                        else if (*p == '|') {
                                // error
                                logerror(syntaxerr);
                                return -1;
                        }
                }
        }
        if (i > 0) {
                ++n;
        }
        return n;
}
/*
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
}*/

int executepipe(char **commands, int n_pipe, int outfd) {
        int cmd = 0;
        int idx = 0;
        int *pipefd = malloc((n_pipe - 1) * 2 * sizeof(int));
        if (pipefd == NULL) {
                logerror(strerror(errno));
                return -1;
        }
        while (cmd < n_pipe) {
                if (cmd < n_pipe - 1) {
                        pipe(pipefd + cmd * 2);
                }
                if (strcmp(commands[idx], "cd") == 0) {
                        if (commands[idx + 1]) {
                                if (chdir(commands[idx + 1]) < 0) {
                                        logerror(strerror(errno));
                                }
                        }
                        else {
                                logerror("no argument");
                        }
                        if (cmd > 0) {
                                close(pipefd[(cmd - 1) * 2]);
                        }
                }
                else if (strcmp(commands[idx], "exit") == 0) {
                        if (cmd > 0) {
                                close(pipefd[(cmd - 1) * 2]);
                        }
                        g_Switch = 0;
                        return 1;
                }
                else if (strcmp(commands[idx], "history") == 0) {
                        if (cmd > 0) {
                                close(pipefd[(cmd - 1) * 2]);
                        }
                        if (cmd < n_pipe - 1) {
                                history(commands + idx, pipefd[2 * cmd + 1]);
                                close(pipefd[2 * cmd + 1]);
                        }
                        else {
                                history(commands + idx, outfd);
                        }
                }
                else {
                        pid_t pid = fork();
                        if (pid < 0) {
                                logerror(strerror(errno));
                        }
                        else if (pid == 0) { // child process
                                if (cmd < n_pipe - 1) {
                                        dup2(pipefd[cmd * 2 + 1], 1);
                                        close(pipefd[cmd * 2]);
                                        close(pipefd[cmd * 2 + 1]);
                                }
                                else {
                                        dup2(outfd, 1);
                                }
                                if (cmd > 0) {
                                        dup2(pipefd[(cmd - 1) * 2], 0);
                                        close(pipefd[(cmd - 1) * 2]);
                                }
                                if (execve(commands[idx], commands + idx, NULL) < 0) {
                                        logerror(strerror(errno));
                                        g_Switch = 0;
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
                                        logerror(strerror(errno));
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

void concurrentpipe(char **commands, int n_pipe, int outfd) {
        int cmd = 0;
        int idx = 0;
        int i, status;
        pid_t *pids;
        int *pipefd = malloc((n_pipe - 1) * 2 * sizeof(int));
        if (pipefd == NULL) {
                logerror(strerror(errno));
                return;
        }
        for (i = 0; i < n_pipe - 1; ++i) {
                pipe(pipefd + 2 * i);
        }
        pids = malloc(n_pipe * sizeof(pid_t));
        if (pids == NULL) {
                free(pipefd);
                logerror(strerror(errno));
                return;
        }
        memset(pids, 0, n_pipe * sizeof(pid_t));
        while (cmd < n_pipe) {
                if (strcmp(commands[idx], "cd") == 0) {
                        if (commands[idx + 1]) {
                                if (chdir(commands[idx + 1]) < 0) {
                                        logerror(strerror(errno));
                                }
                        }
                        else {
                                logerror("no argument");
                        }
                }
                else if (strcmp(commands[idx], "exit") == 0) {
                        g_Switch = 0;
                        free(pipefd);
                        free(pids);
                        return;
                }
                else if (strcmp(commands[idx], "history") == 0) {
                        if (cmd < n_pipe - 1) {
                                history(commands + idx, pipefd[2 * cmd + 1]);
                        }
                        else {
                                history(commands + idx, outfd);
                        }
                }
                else {
                        pids[cmd] = fork();
                        if (pids[cmd] < 0) {
                                logerror(strerror(errno));
                        }
                        else if (pids[cmd] == 0) { // child process
                                if (cmd < n_pipe - 1) {
                                        dup2(pipefd[cmd * 2 + 1], 1);
                                }
                                else {
                                        dup2(outfd, 1);
                                }
                                if (cmd > 0) {
                                        dup2(pipefd[(cmd - 1) * 2], 0);
                                }
                                for (i = 0; i < 2 * (n_pipe - 1); ++i) {
                                        close(pipefd[i]);
                                }
                                if (execve(commands[idx], commands + idx, NULL) < 0) {
                                        logerror(strerror(errno));
                                        g_Switch = 0;
                                        free(pipefd);
                                        free(pids);
                                        return;
                                }
                        }
                }
                while (commands[idx] != NULL) {
                        ++idx;
                }
                ++idx;
                ++cmd;
        }
        for (i = 0; i < 2 * (n_pipe - 1); ++i) {
                close(pipefd[i]);
        }
        free(pipefd);
        pipefd = NULL;
        for (i = 0; i < n_pipe; ++i) {
                if (pids[n_pipe - 1 - i] > 0 && waitpid(pids[n_pipe - 1 - i], &status, 0) < 0) {
                        logerror(strerror(errno));
                }
        }
        free(pids);
        pids = NULL;
}

void history(char **argv, int outfd) {
        int offset;
        if (argv[1] == NULL) {
                int i, qsize = queuesize(&g_Hist);
                for (i = 0; i < qsize; ++i) {
                        char *item = queryqueue(&g_Hist, i);
                        char number[5];
                        sprintf(number, "%d ", i);
                        write(outfd, number, strlen(number));
                        write(outfd, item, strlen(item));
                }
        }
        else if (strcmp(argv[1], "-c") == 0) {
                clearqueue(&g_Hist);
        }
        else if ((offset = str2number(argv[1])) >= 0){
                char *item = queryqueue(&g_Hist, offset);
                if (item == NULL) {
                        logerror("invalid history offset");
                        return;
                }
                size_t num = 5;
                char **commands = malloc(num * sizeof(char*));
                if (commands == NULL) {
                        logerror(strerror(errno));
                        return;
                }
                memset(commands, 0, num * sizeof(char*));
                int n_pipe = parsecommand(item, &commands, &num);
                if (n_pipe > 0) {
                        //executepipe(commands, n_pipe, outfd);
                        concurrentpipe(commands, n_pipe, outfd);
                }
                int i;
                for (i = 0; i < num; ++i) {
                        free(commands[i]);
                        commands[i] = NULL;
                }
                free(commands);
        }
        else {
                logerror("invalid argument");
        }
}

int checkrecursion(char **commands, int n_pipe) {
        int vertex = queuesize(&g_Hist) - 1;
        int cmd = 0;
        int idx = 0;
        int offset;
        while (cmd < n_pipe) {
                if (strcmp(commands[idx], "history") == 0) {
                        if (commands[idx + 1] && (offset = str2number(commands[idx + 1])) >= 0) {
                                setedge(&g_Edge, vertex, offset);
                        }
                }
                while (commands[idx] != NULL) {
                        ++idx;
                }
                ++idx;
                ++cmd;
        }
        return checkcycle(&g_Edge, vertex);
}

void updatehistory(char *str) {
        if (queuefull(&g_Hist)) {
                dequeue(&g_Hist);
                shiftvertex(&g_Edge);
        }
        enqueue(&g_Hist, str);
}

int str2number(char *str) {
        int num = 0;
        char *p;
        for (p = str; *p != '\0'; ++p) {
                if (*p < '0' || *p > '9') {
                        return -1;
                }
                num = num * 10 + *p - '0';
        }
        return num;
}

int logerror(char *str) {
    return fprintf(stderr, "error: %s\n", str);
}
