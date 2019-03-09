#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xdo.h>

int main(int argc, const char *argv[]) {
	xdo_t *xdo = xdo_new(NULL);

	if (xdo == NULL) {
		fprintf(stderr, "xdo_new failed");
		exit(EXIT_FAILURE);
	}

	int n = 1;

	if (argc > 1) {
		n = atoi(argv[1]);
	}
	
	for (int i = 0; i < n; i++) {
		xdo_search_t search;

		memset(&search, 0, sizeof(search));
		search.only_visible = 1;
		search.require = SEARCH_ANY;
		search.searchmask |= 1 << 4; // only visible
		search.searchmask |= 1 << 6; // classname
		search.winclassname = "Gnome-terminal";
		search.max_depth = -1;

		Window *windowlist;
		unsigned int nwindows;

		if (xdo_search_windows(xdo, &search, &windowlist, &nwindows) != 0) {
			fprintf(stderr, "xdo_search_failed");
			exit(EXIT_FAILURE);
		}
		
		free(windowlist);
	}

	xdo_free(xdo);
}
