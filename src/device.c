#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <libfdisk/libfdisk.h>
#include <handy/handy.h>

#include "device.h"

/***********************************************************
 * START OF uprov_device_create_{create,destroy} FUNCTIONS *
 ***********************************************************/

#define BLOCK_DEVICE_MAX_SIZE 50

static int
device_create_with_fdisk (struct uprov_device *device)
{
	int ret = -1;

	struct fdisk_context *cxt = NULL;

	cxt = fdisk_new_context();
	if (!cxt) {
		handy_logme(HANDY_LOG_DANGER, "fdisk_new_context failed");
		return -1;
	}

	ret = fdisk_assign_device(cxt, device->blockDevice, 0);
	if (ret < 0) {
		handy_logme(HANDY_LOG_DANGER, "fdisk_assign_device('%s') failed", device->blockDevice);
		return -1;
	}

	return 0;
}


struct uprov_device *
uprov_device_create (struct uprov_device_create_info *deviceCreateInfo)
{
	int ret = -1;

	struct uprov_device *device = NULL;

	device = calloc(1, sizeof(struct uprov_device));
	if (!device) {
		handy_logme(HANDY_LOG_DANGER, "calloc: %s", strerror(errno));
		return NULL;
	}

	device->blockDevice = strndup(deviceCreateInfo->blockDevice, BLOCK_DEVICE_MAX_SIZE);
	if (!device->blockDevice) {
		handy_logme(HANDY_LOG_DANGER, "strndup: %s", strerror(errno));
		goto uprov_device_create_exit_error;
	}

	ret = device_create_with_fdisk(device);
	if (ret == -1)
		goto uprov_device_create_exit_error;

	return device;

uprov_device_create_exit_error:
	uprov_device_destroy(device);
	return NULL;
}


void
uprov_device_destroy (struct uprov_device *device)
{
	if (!device)
		return;

	free(device->blockDevice);
	free(device->partitions);
	free(device);
}


/*********************************************************
 * END OF uprov_device_create_{create,destroy} FUNCTIONS *
 *********************************************************/
