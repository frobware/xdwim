#ifndef INCLUDE_XDWIM_H
#define INCLUDE_XDWIM_H

#include <unistd.h>
#include <errno.h>

typedef ssize_t (*xdwim_read_fn_t)(int, void *, size_t);

xdwim_read_fn_t xdwim_read = read;

ssize_t xdwim_readln(int fd, char *s, size_t sz) {
	size_t i;
	char c;
	int last_err = 0;
	int newline_read = 0;

	if (s == NULL || sz < 1) {
		errno = EINVAL;
		return -1;
	}

	for (i = 0; i < sz - 1; i++) {
		if (xdwim_read(fd, &c, 1) < 1) {
			last_err = errno;
			break;
		}
		if (c == '\n') {
			newline_read = 1;
			s[i] = '\0';
			break;
		}
		s[i] = c;
	}

	if (!newline_read) {
		// If no characters were read and there was an error
		// with the read system call (last_err != 0), then set
		// errno to last_err, otherwise if no newline was
		// found set errno to ENOBUFS.
		errno = (i == 0 && last_err != 0) ? last_err : ENOBUFS;
		return -1;
	}

	s[i] = '\0';

	return i;
}

#endif	/* INCLUDE_XDWIM_H */
