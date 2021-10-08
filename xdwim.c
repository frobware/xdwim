#define _GNU_SOURCE             /* See feature_test_macros(7) */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>
#include <xdo.h>

#include "xdwim.h"

static void activate_windowclass(xdo_t *xdo, int client_fd, const char *windowclass)
{
	xdo_search_t search;
	unsigned int n_matched_windows;
	Window *matched_windows = NULL;
	Window active_window;
	Window topmost_window;

	memset(&search, 0, sizeof(search));
	search.only_visible = 1;
	search.require = SEARCH_ANY;
	search.searchmask = SEARCH_ONLYVISIBLE | SEARCH_CLASS;
	search.winclass = windowclass;
	search.max_depth = -1;

	if (xdo_search_windows(xdo, &search, &matched_windows, &n_matched_windows) != XDO_SUCCESS) {
		return;
	}

	if (n_matched_windows == 0) {
		goto out;
	}

	if (xdo_get_active_window(xdo, &active_window) != XDO_SUCCESS) {
		goto out;
	}

	topmost_window = matched_windows[n_matched_windows - 1];

	if (topmost_window == active_window && n_matched_windows > 1) {
		topmost_window = matched_windows[n_matched_windows - 2];
	}

	if (xdo_activate_window(xdo, topmost_window) == XDO_SUCCESS) {
		if (xdo_focus_window(xdo, topmost_window) == XDO_SUCCESS) {
			if (write(client_fd, "success\n", 8) != 8) {
				perror("write");
			}
		}
	}

out:
	free(matched_windows);
}

int main()
{
	int listen_fd;
	struct sockaddr_un addr;
	xdo_t *xdo;
	char *windowclass;

	if (signal(SIGPIPE, SIG_IGN) != 0) {
		perror("signal");
		exit(EXIT_FAILURE);
	}

	if ((xdo = xdo_new(NULL)) == NULL) {
		fprintf(stderr, "could not initialise xdo: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path)-1, SOCKPATH_FMT);

	if ((listen_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "error: socket: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	unlink(addr.sun_path);

	if (bind(listen_fd, &addr, sizeof(addr)) != 0) {
		fprintf(stderr, "error: bind(%s): %s\n", addr.sun_path, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (listen(listen_fd, 5) != 0) {
		fprintf(stderr, "error: listen: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	while (1) {
		int cl;
		char line[256] = { '\0' };

		if ((cl = accept4(listen_fd, NULL, NULL, 0)) == -1) {
			fprintf(stderr, "error: accept: %s\n", strerror(errno));
			continue;
		}

		if (readln(cl, line, sizeof(line)) == -1) {
			close(cl);
			continue;
		}

		*strchr(line, '\n') = '\0';

		if ((windowclass = strtok(line, " ")) != NULL) {
			activate_windowclass(xdo, cl, windowclass);
		}

		close(cl);
	}
}
