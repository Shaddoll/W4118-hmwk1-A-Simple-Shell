#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ssize_t getcommand(char **lineptr, size_t *n);
int parsecommand(char *line, char ***commands, size_t *num);

int runshell() {
        char *line = NULL;
        size_t len = 0;
        while(1) {
                ssize_t nread = getcommand(&line, &len);
                if (nread == -1) { // error

                }
                size_t num = 10;
                char **commands = malloc(num * sizeof(char*));
                int r = parsecommand(line, &commands, &num);
                if (r == -1) { // error

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
        return ++n;
}

int executecommand(char **commands, int pipe) {

}
