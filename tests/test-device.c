#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

#include "utils.h"
#include "device.h"

int
main (void)
{
	const struct CMUnitTest tests[] = {
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}