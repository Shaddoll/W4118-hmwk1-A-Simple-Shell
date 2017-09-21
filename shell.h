#ifndef __SHELL_H__
#define __SHELL_H__
#include <sys/types.h>

int runshell(void);
ssize_t getcommand(char **lineptr, size_t *n);
int parsecommand(char *line, char ***commands, size_t *num);
void concurrentpipe(char **commands, int n_pipe, int outfd, int infd, int cfd);
void history(char **argv, int outfd, int infd, int cfd);
void updatehistory(char *str);
int str2number(char *str);
void logerror(char *str);

#endif
