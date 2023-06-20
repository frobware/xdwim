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

#ifndef SOCKPATH_FMT
#define SOCKPATH_FMT "/tmp/xdwim.sock"
#endif

#ifndef MAX_LINE_LEN
#define MAX_LINE_LEN 128
#endif

static int activate_windowclass(xdo_t *xdo, const char *windowclass)
{
	int rc = XDO_ERROR;
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
		return XDO_ERROR;
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
			rc = XDO_SUCCESS;
		}
	}

out:
	free(matched_windows);
	return rc;
}

int main(int argc, char *argv[])
{
	int listen_fd;
	struct sockaddr_un addr;
	xdo_t *xdo;
	char *windowclass;
	struct sigaction act;
	int run_as_daemon = 0;
        int opt;

	if (getenv("DISPLAY") == NULL) {
		fprintf(stderr, "error: no DISPLAY set; exiting.\n");
		exit(EXIT_FAILURE);
	}

	if (getenv("XDG_SESSION_TYPE") != NULL && strcmp("wayland", getenv("XDG_SESSION_TYPE")) == 0) {
		fprintf(stderr, "error: XDG_SESSION_TYPE is wayland; exiting.\n");
		exit(EXIT_FAILURE);
	}

	while ((opt = getopt(argc, argv, "d")) != -1) {
		switch (opt) {
		case 'd':
			run_as_daemon = 1;
			break;
		default:
			fprintf(stderr, "Usage: %s [-d]\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	memset(&act, 0, sizeof(act));
	act.sa_handler = SIG_IGN;
	act.sa_flags = SA_RESTART;
	if (sigaction(SIGPIPE, &act, NULL)) {
		fprintf(stderr, "sigaction: %s", strerror(errno));
		exit(EXIT_FAILURE+2);
	}

	if ((xdo = xdo_new(NULL)) == NULL) {
		fprintf(stderr, "could not initialise xdo: %s\n", strerror(errno));
		exit(EXIT_FAILURE+3);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path)-1, SOCKPATH_FMT);
	unlink(addr.sun_path);

	if ((listen_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "error: socket: %s\n", strerror(errno));
		exit(EXIT_FAILURE+4);
	}

	if (bind(listen_fd, &addr, sizeof(addr)) != 0) {
		fprintf(stderr, "error: bind(%s): %s\n", addr.sun_path, strerror(errno));
		exit(EXIT_FAILURE+5);
	}

	if (listen(listen_fd, 5) != 0) {
		fprintf(stderr, "error: listen: %s\n", strerror(errno));
		exit(EXIT_FAILURE+6);
	}

	if (run_as_daemon) {
		if (daemon(0, 0) != 0) {
			fprintf(stderr, "error: failed to daemonize: %s\n", strerror(errno));
			exit(EXIT_FAILURE+7);
		}
	}

	while (1) {
		int cl;
                char line[MAX_LINE_LEN];

		if ((cl = accept4(listen_fd, NULL, NULL, 0)) == -1) {
			fprintf(stderr, "error: accept: %s\n", strerror(errno));
			continue;
		}

		if (xdwim_readln(cl, line, MAX_LINE_LEN) == -1) {
			perror("getline");
			close(cl);
			continue;
		}

		if ((windowclass = strtok(line, " ")) != NULL) {
			int rc = activate_windowclass(xdo, windowclass);
			if (rc == XDO_SUCCESS) {
				if (write(cl, "success\n", 8) != 8) {
					fprintf(stderr, "error: write: %s\n", strerror(errno));
				}
			} else {
				if (write(cl, "notfound\n", 9) != 9) {
					fprintf(stderr, "error: write: %s\n", strerror(errno));
				}
			}
		}

		close(cl);
	}
}
