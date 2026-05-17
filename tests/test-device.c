#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

#include <udo/udo.h>

#include "device.h"

static void UDO_UNUSED
test_uprov_device_create (void UDO_UNUSED **state)
{
	struct uprov_device *device = NULL;

	struct uprov_device_create_info device_info;
	device_info.block_device = BLOCK_DEVICE;

	device = uprov_device_create(&device_info);
	assert_non_null(device);

	uprov_device_destroy(device);
}


static void UDO_UNUSED
test_uprov_device_resize_with_invalid_device_type (void UDO_UNUSED **state)
{
	int ret = -1;

	struct uprov_device_resize_info device_resize_info;
	device_resize_info.resize.block_device = BLOCK_DEVICE;
	device_resize_info.device_type = UINT32_MAX;
	device_resize_info.part_num = 2;

	ret = uprov_device_resize(&device_resize_info);
	assert_int_equal(ret, -1);
}


static void UDO_UNUSED
test_uprov_device_resize_with_device (void UDO_UNUSED **state)
{
	int ret = -1;

	struct uprov_device *device = NULL;

	struct uprov_device_create_info device_info;
	struct uprov_device_resize_info device_resize_info;

	device_info.block_device = BLOCK_DEVICE;
	device = uprov_device_create(&device_info);
	assert_non_null(device);

	device_resize_info.resize.device = device;
	device_resize_info.device_type = UPROV_DEVICE;
	device_resize_info.part_num = 1;
	ret = uprov_device_resize(&device_resize_info);
	assert_int_equal(ret, 0);

	uprov_device_destroy(device);
}


static void UDO_UNUSED
test_uprov_device_resize_with_block (void UDO_UNUSED **state)
{
	int ret = -1;

	struct uprov_device_resize_info device_resize_info;
	device_resize_info.resize.block_device = BLOCK_DEVICE;
	device_resize_info.device_type = UPROV_DEVICE_BLOCK_DEVICE;
	device_resize_info.part_num = 1;
	ret = uprov_device_resize(&device_resize_info);
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
