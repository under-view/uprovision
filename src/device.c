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
	unsigned int p;

	struct fdisk_context *cxt = NULL;
	struct fdisk_table *table = NULL;
	struct fdisk_partition *part = NULL;

	cxt = fdisk_new_context();
	if (!cxt) {
		handy_logme(HANDY_LOG_DANGER, "fdisk_new_context failed");
		goto device_create_with_fdisk_exit;
	}

	ret = fdisk_assign_device(cxt, device->blockDevice, 0);
	if (ret < 0) {
		handy_logme(HANDY_LOG_DANGER, "fdisk_assign_device('%s') failed", device->blockDevice);
		goto device_create_with_fdisk_exit;
	}

	ret = fdisk_get_partitions(cxt, &table);
	if (ret != 0) {
		handy_logme(HANDY_LOG_DANGER, "fdisk_get_partitions('%s') failed", device->blockDevice);
		goto device_create_with_fdisk_exit;
	}

	device->blockSize = fdisk_get_sector_size(cxt);
	device->partitionCount = fdisk_table_get_nents(table);

	device->partitions = calloc(device->partitionCount, sizeof(struct uprov_device_partition));
	if (!device->partitions) {
		handy_logme(HANDY_LOG_DANGER, "calloc: %s", strerror(errno));
		goto device_create_with_fdisk_exit;
	}

	for (p = 0; p < device->partitionCount; p++) {
		part = fdisk_table_get_partition_by_partno(table, p);

		device->partitions[p].number = fdisk_partition_get_partno(part); // redundant, but fine
		device->partitions[p].startSector = fdisk_partition_get_start(part);
		device->partitions[p].endSector = fdisk_partition_get_end(part);
		device->partitions[p].sectorSize = fdisk_partition_get_size(part);

		fdisk_unref_partition(part); part = NULL;
	}

device_create_with_fdisk_exit:

	if (table)
		fdisk_unref_table(table);
	if (cxt)
		fdisk_unref_context(cxt);

	return ret;
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


/******************************************
 * START OF uprov_device_modify FUNCTIONS *
 ******************************************/


static int
device_modify (struct uprov_device HANDY_UNUSED *device)
{
	return 0;
}


static int
device_modify_with_block (const char *blockDevice)
{
	int ret = -1;

	struct uprov_device *device = NULL;

	struct uprov_device_create_info deviceInfo;
	deviceInfo.blockDevice = blockDevice;

	device = uprov_device_create(&deviceInfo);
	if (!device)
		return -1;

	ret = device_modify(device);	
	uprov_device_destroy(device);

	return ret;
}


int
uprov_device_modify (struct uprov_device_modify_info *deviceModInfo)
{
	int ret = -1;

	switch (deviceModInfo->deviceType) {
		case UPROV_DEVICE:
			ret = device_modify(deviceModInfo->modWith.device);
			break;
		case UPROV_DEVICE_BLOCK_DEVICE:
			ret = device_modify_with_block(deviceModInfo->modWith.blockDevice);
			break;
		default:
			handy_logme(HANDY_LOG_DANGER, "incorrect deviceType specified");
			break;
	}

	return ret;
}


/****************************************
 * END OF uprov_device_modify FUNCTIONS *
 ****************************************/
