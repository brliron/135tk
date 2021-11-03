#include <stdio.h>
#include <string.h>
#include "th175arc.h"

void usage(const char *exe)
{
	printf("Usage: %s -x data.cga [output_dir]\n", exe);
	printf("or:    %s -p data data.cga\n", exe);
}

int main(int argc, const char **argv)
{
	if (argc < 3) {
		usage(argv[0]);
		return 0;
	}

	if (strcmp(argv[1], "-x") == 0) {
		if (argc > 4) {
			usage(argv[0]);
			return 0;
		}
		const char *in_file = argv[2];
		const char *out_dir = NULL;
		if (argc >= 4) {
			out_dir = argv[3];
		}
		return unpack_file(in_file, out_dir);
	}
	else if (strcmp(argv[1], "-p") == 0) {
		if (argc != 4) {
			usage(argv[0]);
			return 0;
		}
		const char *in_dir    = argv[2];
		const char *out_file  = argv[3];
		return repack_file(in_dir, out_file);
	}
	else {
		usage(argv[0]);
	}
}
