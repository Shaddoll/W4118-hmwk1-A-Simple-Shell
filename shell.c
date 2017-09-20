#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include "queue.h"
#include "graph.h"
#include "shell.h"

Queue g_Hist;
Graph g_Edge;
int g_Switch;

int runshell(void)
{
	char *line = NULL;
	size_t len = 0;
	size_t num = 5;
	char **commands = malloc(num * sizeof(char *));

	if (commands == NULL) {
		logerror(strerror(errno));
		return 1;
	}
	memset(commands, 0, num * sizeof(char *));
	initqueue(&g_Hist);
	initgraph(&g_Edge);
	g_Switch = 1;
	while (g_Switch) {
		ssize_t n_read;
		int n_pipe, i;

		n_read = getcommand(&line, &len);
		if (n_read < 0)
			break;
		if (n_read > 0 && line[0] != '\n')
			updatehistory(line);
		n_pipe = parsecommand(line, &commands, &num);
		if (n_pipe > 0) {
			removerecursion(commands, n_pipe);
			concurrentpipe(commands, n_pipe, 1);
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

ssize_t getcommand(char **lineptr, size_t *n)
{
	printf("$");
	return getline(lineptr, n, stdin);
}

int parsecommand(char *line, char ***commands, size_t *num)
{
	int n = 0;
	int i = 0;
	char *p = line;
	char *syntaxerr = "syntax error near unexpected token `|`";

	while (*p == ' ')
		++p;
	if (*p == '|') {
		logerror(syntaxerr);
		return -1;
	}
	while (*p != '\n') {
		char *q = p;

		while (*q != ' ' && *q != '|' && *q != '\n')
			++q;
		(*commands)[i] = malloc(q - p + 1);
		if ((*commands)[i] == NULL) {
			logerror(strerror(errno));
			return -1;
		}
		strncpy((*commands)[i], p, q - p);
		(*commands)[i++][q - p] = '\0';
		if (i >= *num - 1) {
			char **temp;

			temp = realloc(*commands, *num * 2 * sizeof(char **));
			if (temp == NULL) {
				logerror(strerror(errno));
				return -1;
			}
			memset(temp + *num, 0, *num * sizeof(char **));
			*commands = temp;
			*num *= 2;
		}
		while (*q == ' ')
			++q;
		p = q;
		if (*q == '|') {
			++n;
			(*commands)[i++] = NULL;
			if (i >= *num - 1) {
				char **temp;

				temp = realloc(*commands, *num * 2 * sizeof(char **));
				if (temp == NULL)
					return -1;
				memset(temp + *num, 0, *num * sizeof(char **));
				*commands = temp;
				*num *= 2;
			}
			p = q + 1;
			while (*p == ' ')
				++p;
			if (*p == '\n' || *p == '|') {
				logerror(syntaxerr);
				return -1;
			}
		}
	}
	if (i > 0)
		++n;
	return n;
}

void concurrentpipe(char **commands, int n_pipe, int outfd)
{
	int cmd = 0;
	int idx = 0;
	int i;
	pid_t *pids;
	int *pipefd = malloc((n_pipe - 1) * 2 * sizeof(int));

	if (pipefd == NULL && n_pipe != 1) {
		logerror(strerror(errno));
		return;
	}
	for (i = 0; i < n_pipe - 1; ++i)
		pipe(pipefd + 2 * i);
	pids = malloc(n_pipe * sizeof(pid_t));
	if (pids == NULL) {
		free(pipefd);
		logerror(strerror(errno));
		return;
	}
	memset(pids, 0, n_pipe * sizeof(pid_t));
	while (cmd < n_pipe) {
		int output = cmd < n_pipe - 1 ? pipefd[cmd * 2 + 1] : outfd;
		int input = cmd > 0 ? pipefd[(cmd - 1) * 2] : 0;

		if (strcmp(commands[idx], "cd") == 0) {
			if (commands[idx + 1] == NULL)
				logerror("no argument");
			else if (chdir(commands[idx + 1]) < 0)
				logerror(strerror(errno));
		} else if (strcmp(commands[idx], "exit") == 0) {
			g_Switch = 0;
			free(pipefd);
			free(pids);
			return;
		} else if (strcmp(commands[idx], "history") == 0) {
			history(commands + idx, output);
		} else {
			pids[cmd] = fork();
			if (pids[cmd] < 0)
				logerror(strerror(errno));
			else if (pids[cmd] == 0) {
				dup2(output, 1);
				dup2(input, 0);
				for (i = 0; i < 2 * (n_pipe - 1); ++i)
					close(pipefd[i]);
				if (execv(commands[idx], commands + idx) < 0) {
					logerror(strerror(errno));
					g_Switch = 0;
					free(pipefd);
					free(pids);
					return;
				}
			}
		}
		while (commands[idx] != NULL)
			++idx;
		++idx;
		++cmd;
	}
	for (i = 0; i < 2 * (n_pipe - 1); ++i)
		close(pipefd[i]);
	free(pipefd);
	pipefd = NULL;
	for (i = 0; i < n_pipe; ++i) {
		if (pids[i] > 0 && waitpid(pids[i], NULL, 0) < 0)
			logerror(strerror(errno));
	}
	free(pids);
	pids = NULL;
}

void history(char **argv, int outfd)
{
	int offset = str2number(argv[1]);

	if (argv[1] == NULL) {
		int i, qsize = queuesize(&g_Hist);

		for (i = 0; i < qsize; ++i) {
			char *item = queryqueue(&g_Hist, i);
			char number[5];

			sprintf(number, "%d ", i);
			write(outfd, number, strlen(number));
			write(outfd, item, strlen(item));
		}
	} else if (strcmp(argv[1], "-c") == 0) {
		clearqueue(&g_Hist);
		cleargraph(&g_Edge);
	} else if (offset >= 0) {
		char *item = queryqueue(&g_Hist, offset);

		if (item == NULL) {
			logerror("event not found");
			return;
		}
		size_t num = 5;
		char **commands = malloc(num * sizeof(char *));

		if (commands == NULL) {
			logerror(strerror(errno));
			return;
		}
		memset(commands, 0, num * sizeof(char *));
		int n_pipe = parsecommand(item, &commands, &num);

		if (n_pipe > 0)
			concurrentpipe(commands, n_pipe, outfd);
		int i;

		for (i = 0; i < num; ++i) {
			free(commands[i]);
			commands[i] = NULL;
		}
		free(commands);
	} else if (argv[1][0] == '\0') {
		logerror("event cause infinite recursion");
	} else {
		logerror("invalid argument");
	}
}

void removerecursion(char **commands, int n_pipe)
{
	int cmd = 0;
	int idx = 0;
	int offset = -1;
	int vertex = queuesize(&g_Hist) - 1;

	while (cmd < n_pipe) {
		if (strcmp(commands[idx], "history") == 0) {
			//if (commands[idx + 1] && strcmp(commands[idx + 1], "-c") == 0)
			//	return;
			offset = str2number(commands[idx + 1]);
			setedge(&g_Edge, vertex, offset);
			if (offset >= 0 && checkcycle(&g_Edge, offset))
				commands[idx + 1][0] = '\0';
		}
		while (commands[idx] != NULL)
			++idx;
		++idx;
		++cmd;
	}
}

void updatehistory(char *str)
{
	if (queuefull(&g_Hist)) {
		dequeue(&g_Hist);
		shiftvertex(&g_Edge);
	}
	enqueue(&g_Hist, str);
}

int str2number(char *str)
{
	if (str == NULL || strlen(str) == 0)
		return -1;
	int num = 0;
	char *p;

	for (p = str; *p != '\0'; ++p) {
		if (*p < '0' || *p > '9')
			return -1;
		num = num * 10 + *p - '0';
	}
	return num;
}

int logerror(char *str)
{
	return fprintf(stderr, "error: %s\n", str);
}
