#include <stdlib.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "xdwim.h"

#define NELEMENTS(A) ((sizeof (A)) / (sizeof (A[0])))
#define MAX_BUFFER_SIZE 128

struct test_case {
	const char* input;
	const char* expected_content;
	int expected_return;
	size_t line_length;
	xdwim_read_fn_t mock_read;
};

static char *ebad_fd_buf = "ebad_fd_read\n";

ssize_t ebad_fd_read(int fd, void *buf, size_t count) {
	if (*ebad_fd_buf == '\n') {
		errno = EBADF;
		return -1;
	}
	*((char *)buf) = *ebad_fd_buf++;
	return 1;
}

struct test_case test_cases[] = {
	{"", NULL, -1, MAX_BUFFER_SIZE, NULL},
	{"\0", NULL, -1, MAX_BUFFER_SIZE, NULL},
	{"\n", "", 0, MAX_BUFFER_SIZE, NULL},
	{"a", NULL, -1, MAX_BUFFER_SIZE, NULL},
	{"an embedded null before the new line\0\n", NULL, -1, MAX_BUFFER_SIZE, NULL},
	{"buffer overflow\n", NULL, -1, 5, NULL},
	{"ebad_fd_read\n", NULL, -1, MAX_BUFFER_SIZE, ebad_fd_read}, /* mock read error */
	{"first\nline\n", "first", 5, MAX_BUFFER_SIZE, NULL},
	{"hello, world!\n", "hello, world!", 13, MAX_BUFFER_SIZE, NULL},
	{"negative buf size", NULL, -1, -1, NULL},
	{"no newline", NULL, -1, MAX_BUFFER_SIZE, NULL},
	{"zero buf size", NULL, -1, 0, NULL},
	{NULL, NULL, -1, -1, NULL}, /* NULL input buffer */
};

void test_readln() {
	for (size_t i = 0; i < NELEMENTS(test_cases); i++) {
		int rc;
		int fd[2];
		size_t l;
		ssize_t n;
		char line[MAX_BUFFER_SIZE];

		rc = pipe(fd);
		CU_ASSERT_EQUAL_FATAL(rc, 0);

		if (test_cases[i].input != NULL) {
			l = strlen(test_cases[i].input);
			n = write(fd[1], test_cases[i].input, l);
			CU_ASSERT_EQUAL_FATAL(n, (ssize_t)l);
		};

		rc = close(fd[1]);
		CU_ASSERT_EQUAL_FATAL(rc, 0);

		xdwim_read_fn_t original_read = xdwim_read;
		if (test_cases[i].mock_read != NULL) {
			xdwim_read = test_cases[i].mock_read;
		}

		rc = xdwim_readln(fd[0], test_cases[i].input ? line : NULL, test_cases[i].line_length);
                xdwim_read = original_read;

		CU_ASSERT_EQUAL_FATAL(rc, test_cases[i].expected_return);
		if (test_cases[i].expected_return != -1) {
			CU_ASSERT_STRING_EQUAL_FATAL(line, test_cases[i].expected_content);
		}

		rc = close(fd[0]);
		CU_ASSERT_EQUAL_FATAL(rc, 0);
	}
}

int main()
{
	struct CU_Suite* suite = NULL;

	if (CU_initialize_registry() != CUE_SUCCESS)
		return CU_get_error();

	if ((suite = CU_add_suite("readln_test_suite", NULL, NULL)) == NULL) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	if (CU_add_test(suite, "test_readln", test_readln) == NULL) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();

	int nfailures = CU_get_number_of_failures();
	CU_cleanup_registry();
	return (nfailures > 0) ? EXIT_FAILURE : EXIT_SUCCESS;
}
