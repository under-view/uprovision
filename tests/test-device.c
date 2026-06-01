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

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

#include <udo/udo.h>

#include "device.h"

/*****************************************
 * Start of test_device_create functions *
 *****************************************/

static void UDO_UNUSED
test_device_create (void UDO_UNUSED **state)
{
	struct uprov_device *device = NULL;

	udo_log_set_level(UDO_LOG_ALL);

	device = uprov_device_create(NULL, BLOCK_DEVICE);
	assert_non_null(device);

	uprov_device_destroy(device);
}

/***************************************
 * End of test_device_create functions *
 ***************************************/


/*********************************************
 * Start of test_device_get_sizeof functions *
 *********************************************/

static void UDO_UNUSED
test_device_get_sizeof (void UDO_UNUSED **state)
{
	int size = 0;
	size = uprov_device_get_sizeof();
	assert_int_not_equal(size, 0);
}

/*******************************************
 * End of test_device_get_sizeof functions *
 *******************************************/

int
main (void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_device_create),
		cmocka_unit_test(test_device_get_sizeof),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
