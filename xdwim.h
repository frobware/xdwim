#ifndef INCLUDE_XDWIM_H
#define INCLUDE_XDWIM_H

#include <unistd.h>

#define SOCKPATH_FMT "/tmp/xdwim.sock"

static inline int readln(int fd, char *s, size_t sz) {
	size_t i;

	for (i = 0; i < sz; i++) {
		int rc;
		if ((rc = read(fd, &s[i], 1)) != 1) {
			return rc;
		}
		if (s[i] == '\n' || s[i] == '\0') {
			break;
		}
	}

	if (s[i] == '\n') {
		if (i+1 < sz) {
			s[i+1] = '\0';
			return 0;
		}
	}

	return -1;
}

#endif /* INCLUDE_XDWIM_H */
