#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

#include <udo/udo.h>

#include "uprov.h"

static void
help_msg (const char *binName)
{
	udo_log_print(UDO_LOG_SUCCESS, "Usage: %s <options>\n", binName);
	udo_log_print(UDO_LOG_WARNING, "Example: %s --resize 1\n", binName);
	udo_log_print(UDO_LOG_INFO, "Options:\n");
	udo_log_print(UDO_LOG_ERROR, "\t-r,--resize <partition number> "); udo_log_print(UDO_LOG_INFO, "Resize a drive partition to extend\n");
	udo_log_print(UDO_LOG_INFO,   "\t                               "); udo_log_print(UDO_LOG_INFO, "it out to the end of drive.\n");
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
		udo_log_print(UDO_LOG_ERROR, "[x] No option specified!!\n\n");
		help_msg(argv[0]);
	}

	// Allows us to control what's displayed upon invalid option.
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
				udo_log_print(UDO_LOG_ERROR, "[x] Invalid option specified!!\n\n");
				help_msg(argv[0]);
				break;
			default:
				help_msg(argv[0]);
				break;
		}
	}

	return 0;
}
