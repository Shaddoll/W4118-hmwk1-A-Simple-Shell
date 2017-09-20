#ifndef __SHELL_H__
#define __SHELL_H__
#include <sys/types.h>

int runshell(void);
ssize_t getcommand(char **lineptr, size_t *n);
int parsecommand(char *line, char ***commands, size_t *num);
int executecommand(char **commands, int pipe);
int executepipe(char **commands, int n_pipe, int outfd);
void concurrentpipe(char **commands, int n_pipe, int outfd);
void history(char **argv, int fd);
void removerecursion(char **commands, int n_pipe);
void updatehistory(char *str);
int str2number(char *str);
int logerror(char *str);

#endif
