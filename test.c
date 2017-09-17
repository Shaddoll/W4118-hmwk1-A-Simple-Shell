int eval(char **commands, int infd, int outfd) {
	if (strcmp(commands[0], "cd") == 0) {
		if (commands[1] == NULL)
			logerror("no arguemnt");
		else if (chdir(commands[1]) < 0)
			logerror(strerror(errno));
	} else if (strcmp(commands[0], "exit")) {
		g_Switch = 0;
	} else if (strcmp(commands[0], "history") == 0) {
		history(commands, outfd);
	} else {
		pid_t pid = fork();
		if (pid < 0) {
			logerror(strerror(errno));
		}
		else if (pid == 0) {
			dup2(outfd, 1);
			dup2(infd, 0);
			execv(commands[0], commands);
			logerror(strerror(errno));
			exit(1);
		}
		return pid;
	}
	return 0;
}
