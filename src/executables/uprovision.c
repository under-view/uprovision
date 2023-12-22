#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include <handy/handy.h>

#include "uprov.h"

static void
help_msg (const char *binName)
{
	fprintf(stdout, "Usage: %s <options>\n", binName);
	fprintf(stdout, "Options:\n");
	fprintf(stdout, "\t-r,--resize <partition number> "); fprintf(stdout, "Resize a drive partition to extend\n");
	fprintf(stdout, "\t                               "); fprintf(stdout, "it out to the end of drive.\n");
	exit(EXIT_FAILURE);
}


int
main (int argc, char *argv[])
{
	int c;
	int optIndex = 0;

	static struct option options[] = {
		{"resize",  required_argument, 0, 'r' },
		{"help",    no_argument,       0, 'h' },
		{0,         0,                 0,  0  }
	};

	if (argc < 2) {
		fprintf(stdout, "[x] No option specified.\n");
		help_msg(argv[0]);
	}

	opterr = 0;

	while (1) {
		c = getopt_long(argc, argv, "hr:", options, &optIndex);
		if (c == -1)
			break;

		switch (c) {
			case 'r':
				int partNum = atoi(optarg);
				fprintf(stdout, "Resizeing partition %d\n", partNum);
				break;
			case 'h':
				help_msg(argv[0]);
				break;
			case '?':
				fprintf(stdout, "[x] Invalid option\n");
				help_msg(argv[0]);
				break;
			default:
				help_msg(argv[0]);
				break;
		}
	}

	return 0;
}
