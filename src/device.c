#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <libfdisk/libfdisk.h>
#include <udo/udo.h>

#include "device.h"

#define BLK_NAME_MAX (1<<5)
#define PARTITIONS_MAX (1<<7)

struct uprov_device_part
{
	long int number;
	long int start_sector;
	long int end_sector;
	long int sector_size;
};


struct uprov_device
{
	struct udo_log_error_struct err;
	bool                        free;
	struct uprov_device_part    parts[PARTITIONS_MAX];
	char                        block_device[BLK_NAME_MAX];
	int                         bdev_fd;
	unsigned int                block_size;
	unsigned int                part_count;
};

/*****************************************
 * Start of global to C source functions *
 *****************************************/

struct p_uprov_fdisk
{
	struct fdisk_context *ctx;
	struct fdisk_table   *table;
};


static void
p_uprov_fdisk_destroy (struct p_uprov_fdisk *fdisk)
{
	if (fdisk->table)
		fdisk_unref_table(fdisk->table);
	if (fdisk->ctx) {
		fdisk_deassign_device(fdisk->ctx, 1);
		fdisk_unref_context(fdisk->ctx);
	}
}

/***************************************
 * End of global to C source functions *
 ***************************************/


/***************************************
 * Start uprov_device_create functions *
 ***************************************/

#define BLOCK_DEVICE_MAX_SIZE 50

static int
device_create_with_fdisk (struct uprov_device *device)
{
	int err = -1;
	unsigned int p;

	struct p_uprov_fdisk fdisk;

	struct fdisk_partition *part = NULL;

	memset(&fdisk, 0, sizeof(struct p_uprov_fdisk));

	fdisk.ctx = fdisk_new_context();
	if (!(fdisk.ctx)) {
		udo_log_error("fdisk_new_context failed\n");
		return -1;
	}

	device->bdev_fd = open(device->block_device, O_RDWR);
	if (device->bdev_fd < 0) {
		udo_log_error("open: %s\n", strerror(errno));
		p_uprov_fdisk_destroy(&fdisk);
		return -1;
	}

	err = fdisk_assign_device_by_fd(fdisk.ctx, \
		device->bdev_fd, device->block_device, 0);
	if (err < 0) {
		udo_log_error("fdisk_assign_device_by_fd('%d','%s') failed\n",
		              device->bdev_fd, device->block_device);
		p_uprov_fdisk_destroy(&fdisk);
		return -1;
	}

	err = fdisk_get_partitions(fdisk.ctx, &(fdisk.table));
	if (err != 0) {
		udo_log_error("fdisk_get_partitions('%d','%s') failed\n",
		              device->bdev_fd, device->block_device);
		p_uprov_fdisk_destroy(&fdisk);
		return -1;
	}

	device->block_size = fdisk_get_sector_size(fdisk.ctx);
	device->part_count = fdisk_table_get_nents(fdisk.table);

	for (p = 0; p < device->part_count; p++) {
		part = fdisk_table_get_partition_by_partno(fdisk.table, p);

		device->parts[p].number = fdisk_partition_get_partno(part); // redundant, but fine
		device->parts[p].start_sector = fdisk_partition_get_start(part);
		device->parts[p].end_sector = fdisk_partition_get_end(part);
		device->parts[p].sector_size = fdisk_partition_get_size(part);

		fdisk_unref_partition(part); part = NULL;
	}

	p_uprov_fdisk_destroy(&fdisk);

	return 0;
}


struct uprov_device *
uprov_device_create (struct uprov_device *p_device,
                     const void *p_device_info)
{
	int ret = -1;

	struct uprov_device *device = p_device;
	const struct uprov_device_create_info *device_info = p_device_info;

	if (!device) {
		device = calloc(1, sizeof(struct uprov_device));
		if (!device) {
			udo_log_error("calloc: %s\n", strerror(errno));
			return NULL;
		}
	}

	if (device_info->block_device) {
		strncpy(&(device->block_device[0]), \
			device_info->block_device, \
			BLK_NAME_MAX-1);
	}

	ret = device_create_with_fdisk(device);
	if (ret == -1) {
		uprov_device_destroy(device);
		return NULL;
	}

	return device;
}

/*************************************
 * End uprov_device_create functions *
 *************************************/


/******************************************
 * Start of uprov_device_resize functions *
 ******************************************/

static int
device_resize (struct uprov_device *device, int part_num)
{
	int err = -1;

	uint64_t end_sector = 0;
	uint64_t start_sector = 0;
	uint64_t total_sectors = 0;
	uint64_t offset_sectors = 0;

	struct fdisk_partition *part = NULL;

	struct p_uprov_fdisk fdisk;
	struct uprov_device_part partition;

	partition = device->parts[part_num];

	memset(&fdisk, 0, sizeof(struct p_uprov_fdisk));

	fdisk.ctx = fdisk_new_context();
	if (!(fdisk.ctx)) {
		udo_log_error("fdisk_new_context failed\n");
		return -1;
	}

	err = fdisk_assign_device_by_fd(fdisk.ctx, \
		device->bdev_fd, device->block_device, 0);
	if (err < 0) {
		udo_log_error("fdisk_assign_device_by_fd('%d','%s') failed\n",
		              device->bdev_fd, device->block_device);
		p_uprov_fdisk_destroy(&fdisk);
		return -1;
	}

	fdisk_disable_dialogs(fdisk.ctx, 1);

	err = fdisk_get_partitions(fdisk.ctx, &(fdisk.table));
	if (err != 0) {
		udo_log_error("fdisk_get_partitions('%d','%d') failed\n",
                              device->bdev_fd, device->block_device);
		p_uprov_fdisk_destroy(&fdisk);
		return -1;
	}

	start_sector = partition.start_sector;
	end_sector = partition.end_sector;
	total_sectors = fdisk_get_nsectors(fdisk.ctx);
	offset_sectors = total_sectors - start_sector - 512;

	if (total_sectors > end_sector) {
		part = fdisk_table_get_partition_by_partno(fdisk.table, partition.number);

		fdisk_partition_size_explicit(part, 1);
		fdisk_partition_set_size(part, offset_sectors);
		fdisk_partition_end_follow_default(part, 1);

		err = fdisk_set_partition(fdisk.ctx, partition.number, part);
		if (err != 0) {
			udo_log_error("fdisk_set_partition('%d','%s') failed\n",
			              device->bdev_fd, device->block_device);
			p_uprov_fdisk_destroy(&fdisk);
			return -1;
		}

		fdisk_unref_partition(part); part = NULL;
	}

	p_uprov_fdisk_destroy(&fdisk);

	return 0;
}


static int
device_resize_with_block (const char *block_device, int part_num)
{
	int ret = -1;

	struct uprov_device *device = NULL;

	struct uprov_device_create_info device_info;
	device_info.block_device = block_device;

	device = uprov_device_create(NULL, &device_info);
	if (!device)
		return -1;

	ret = device_resize(device, part_num);
	uprov_device_destroy(device);

	return ret;
}


int
uprov_device_resize (struct uprov_device_resize_info *device_info)
{
	int ret = -1;

	switch (device_info->device_type) {
		case UPROV_DEVICE:
			ret = device_resize(device_info->resize.device, device_info->part_num);
			break;
		case UPROV_DEVICE_BLOCK_DEVICE:
			ret = device_resize_with_block(device_info->resize.block_device, device_info->part_num);
			break;
		default:
			udo_log_error("Incorrect @device_type specified\n");
			break;
	}

	return ret;
}

/****************************************
 * End of uprov_device_resize functions *
 ****************************************/


/****************************************
 * Start uprov_device_destroy functions *
 ****************************************/

void
uprov_device_destroy (struct uprov_device *device)
{
	if (!device)
		return;

	close(device->bdev_fd);

	if (device->free) {
		free(device);
	} else {
		memset(device, 0, sizeof(struct uprov_device));
	}
}

/**************************************
 * End uprov_device_destroy functions *
 **************************************/


/****************************************************
 * Start of non struct uprov_device param functions *
 ****************************************************/

int
uprov_device_get_sizeof (void)
{
	return sizeof(struct uprov_device);
}

/**************************************************
 * End of non struct uprov_device param functions *
 **************************************************/
