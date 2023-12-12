#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

#include <handy/handy.h>

#include "device.h"

static void HANDY_UNUSED
test_uprov_device_create (void HANDY_UNUSED **state)
{
	struct uprov_device *device = NULL;

	struct uprov_device_create_info deviceInfo;
	deviceInfo.blockDevice = BLOCK_DEVICE;

	device = uprov_device_create(&deviceInfo);
	assert_non_null(device);

	uprov_device_destroy(device);
}


int
main (void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_uprov_device_create),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
