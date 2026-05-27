/*
 * MIT License
 *
 * Copyright (c) 2023-2026 Underview
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

#include <udo/udo.h>

#include "uprov.h"

static void
help_msg (const char *bin_name)
{
	udo_log_print(UDO_LOG_SUCCESS, "Usage: %s <options>\n", bin_name);
	udo_log_print(UDO_LOG_WARNING, "Example: %s --resize 1\n", bin_name);
	udo_log_print(UDO_LOG_INFO,    "Options:\n");
	udo_log_print(UDO_LOG_ERROR,   "\t-r,--resize <partition number> "); udo_log_print(UDO_LOG_INFO, "Resize a drive partition to extend\n");
	udo_log_print(UDO_LOG_INFO,    "\t                               "); udo_log_print(UDO_LOG_INFO, "it out to the end of drive.\n");
	exit(EXIT_FAILURE);
}


int
main (int argc, char *argv[])
{
	int c;
	int opt_index = 0;

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
		c = getopt_long(argc, argv, "hr:", options, &opt_index);
		if (c == -1)
			break;

		switch (c) {
			case 'r':
				int part_num = atoi(optarg);
				fprintf(stdout, "Resizeing partition %d\n", part_num);
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
