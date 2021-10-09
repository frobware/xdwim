#define _GNU_SOURCE             /* See feature_test_macros(7) */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>

#include "xdwim.h"

static char *mprintf(const char *fmt, ...)
{
	size_t size = 0;
	char *p = NULL;
	va_list ap;

	/* Determine required size. */
	va_start(ap, fmt);
	size = vsnprintf(p, size, fmt, ap);
	va_end(ap);

	size++; /* For '\0' */

	if ((p = malloc(size)) == NULL) {
		return NULL;
	}

	va_start(ap, fmt);
	size = vsnprintf(p, size, fmt, ap);
	va_end(ap);

	return p;
}

int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_un addr;
	char *windowclass;
	char line[32] = { '\0' };

	if (argc < 3) {
		fprintf(stderr, "usage: <windowclass> <program>\n");
		exit(EXIT_FAILURE);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path)-1, SOCKPATH_FMT);

	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket error");
		exit(EXIT_FAILURE);
	}

	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("connect");
		exit(EXIT_FAILURE);
	}

	if ((windowclass = mprintf("%s\n", argv[1])) == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	if (write(sockfd, windowclass, strlen(windowclass)) != strlen(windowclass)) {
		perror("write");
		exit(EXIT_FAILURE);
	}

	free(windowclass);

	if (readln(sockfd, line, sizeof(line)) == -1) {
		perror("read");
		exit(EXIT_FAILURE);
	}

	if (strcmp(line, "success\n") != 0) {
		int pid = fork();
		if (pid == 0) {
			if (execvpe(argv[2], &argv[3], environ) == -1) {
				perror("execve");
				exit(EXIT_FAILURE);
			}
		} else if (pid == -1) {
			perror("fork");
			exit(EXIT_SUCCESS);
		}
	}

	close(sockfd);
	exit(EXIT_SUCCESS);
}
