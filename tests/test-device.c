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


static void HANDY_UNUSED
test_uprov_device_resize_with_invalid_device_type (void HANDY_UNUSED **state)
{
	int ret = -1;

	struct uprov_device_resize_info deviceResizeInfo;
	deviceResizeInfo.resize.blockDevice = BLOCK_DEVICE;
	deviceResizeInfo.deviceType = UINT32_MAX;
	deviceResizeInfo.partNum = 2;

	ret = uprov_device_resize(&deviceResizeInfo);
	assert_int_equal(ret, -1);
}


static void HANDY_UNUSED
test_uprov_device_resize_with_device (void HANDY_UNUSED **state)
{
	int ret = -1;

	struct uprov_device *device = NULL;

	struct uprov_device_create_info deviceInfo;
	struct uprov_device_resize_info deviceResizeInfo;

	deviceInfo.blockDevice = BLOCK_DEVICE;
	device = uprov_device_create(&deviceInfo);
	assert_non_null(device);

	deviceResizeInfo.resize.device = device;
	deviceResizeInfo.deviceType = UPROV_DEVICE;
	deviceResizeInfo.partNum = 1;
	ret = uprov_device_resize(&deviceResizeInfo);
	assert_int_equal(ret, 0);

	uprov_device_destroy(device);
}


static void HANDY_UNUSED
test_uprov_device_resize_with_block (void HANDY_UNUSED **state)
{
	int ret = -1;

	struct uprov_device_resize_info deviceResizeInfo;
	deviceResizeInfo.resize.blockDevice = BLOCK_DEVICE;
	deviceResizeInfo.deviceType = UPROV_DEVICE_BLOCK_DEVICE;
	deviceResizeInfo.partNum = 1;
	ret = uprov_device_resize(&deviceResizeInfo);
	assert_int_equal(ret, 0);
}


int
main (void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_uprov_device_create),
		cmocka_unit_test(test_uprov_device_resize_with_invalid_device_type),
		cmocka_unit_test(test_uprov_device_resize_with_device),
		cmocka_unit_test(test_uprov_device_resize_with_block),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
