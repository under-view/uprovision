#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
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
		handy_logme_err("fdisk_new_context failed");
		goto device_create_with_fdisk_exit;
	}

	device->fdiskContext = cxt;
	device->blockDeviceFd = open(device->blockDevice, O_RDWR);
	if (device->blockDeviceFd < 0) {
		handy_logme_err("open: %s", strerror(errno));
		goto device_create_with_fdisk_exit;
	}

	ret = fdisk_assign_device_by_fd(cxt, device->blockDeviceFd, device->blockDevice, 0);
	if (ret < 0) {
		handy_logme_err("fdisk_assign_device_by_fd('%d','%s') failed",
		                device->blockDeviceFd, device->blockDevice);
		goto device_create_with_fdisk_exit;
	}

	ret = fdisk_get_partitions(cxt, &table);
	if (ret != 0) {
		handy_logme_err("fdisk_get_partitions('%s') failed", device->blockDevice);
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

	return ret;
}


struct uprov_device *
uprov_device_create (struct uprov_device_create_info *deviceCreateInfo)
{
	int ret = -1;

	struct uprov_device *device = NULL;

	device = calloc(1, sizeof(struct uprov_device));
	if (!device) {
		handy_logme_err("calloc: %s", strerror(errno));
		return NULL;
	}

	device->blockDevice = strndup(deviceCreateInfo->blockDevice, BLOCK_DEVICE_MAX_SIZE);
	if (!device->blockDevice) {
		handy_logme_err("strndup: %s", strerror(errno));
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

	close(device->blockDeviceFd);
	free(device->blockDevice);
	free(device->partitions);
	fdisk_deassign_device((struct fdisk_context*)device->fdiskContext, 0);
	fdisk_unref_context((struct fdisk_context*)device->fdiskContext);
	free(device);
}


/*********************************************************
 * END OF uprov_device_create_{create,destroy} FUNCTIONS *
 *********************************************************/


/******************************************
 * START OF uprov_device_resize FUNCTIONS *
 ******************************************/


static int
device_resize (struct uprov_device *device, int partNum)
{
	int ret = -1;

	uint64_t totalSectors = 0;
	uint64_t startSector = 0;
	uint64_t endSector = 0;
	uint64_t offsetSectors = 0;

	struct fdisk_context *cxt = NULL;
	struct fdisk_table *table = NULL;
	struct fdisk_partition *part = NULL;
	struct uprov_device_partition partition;

	cxt = device->fdiskContext;
	partition = device->partitions[partNum];

	fdisk_disable_dialogs(cxt, 1);

	ret = fdisk_get_partitions(cxt, &table);
	if (ret != 0) {
		handy_logme_err("fdisk_get_partitions('%d','%d') failed",
		                device->blockDeviceFd, device->blockDevice);
		goto device_resize_exit;
	}

	startSector = partition.startSector;
	endSector = partition.endSector;
	totalSectors = fdisk_get_nsectors(cxt);
	offsetSectors = totalSectors - startSector - 512;

	if (totalSectors > endSector) {
		part = fdisk_table_get_partition_by_partno(table, partition.number);

		fdisk_partition_size_explicit(part, 1);
		fdisk_partition_set_size(part, offsetSectors);
		fdisk_partition_end_follow_default(part, 1);

		ret = fdisk_set_partition(cxt, partition.number, part);
		if (ret != 0) {
			handy_logme_err("fdisk_set_partition('%d','%s') failed",
					device->blockDeviceFd, device->blockDevice);
		}

		fdisk_unref_partition(part); part = NULL;
	}

device_resize_exit:

	if (table)
		fdisk_unref_table(table);

	fdisk_disable_dialogs(cxt, 0);

	return ret;
}


static int
device_resize_with_block (const char *blockDevice, int partNum)
{
	int ret = -1;

	struct uprov_device *device = NULL;

	struct uprov_device_create_info deviceInfo;
	deviceInfo.blockDevice = blockDevice;

	device = uprov_device_create(&deviceInfo);
	if (!device)
		return -1;

	ret = device_resize(device, partNum);
	uprov_device_destroy(device);

	return ret;
}


int
uprov_device_resize (struct uprov_device_resize_info *deviceResizeInfo)
{
	int ret = -1;

	switch (deviceResizeInfo->deviceType) {
		case UPROV_DEVICE:
			ret = device_resize(deviceResizeInfo->resize.device, deviceResizeInfo->partNum);
			break;
		case UPROV_DEVICE_BLOCK_DEVICE:
			ret = device_resize_with_block(deviceResizeInfo->resize.blockDevice, deviceResizeInfo->partNum);
			break;
		default:
			handy_logme_err("incorrect deviceType specified");
			break;
	}

	return ret;
}


/****************************************
 * END OF uprov_device_resize FUNCTIONS *
 ****************************************/
