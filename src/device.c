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
p_device_create_with_fdisk (struct uprov_device *device)
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
                     const char *block_device)
{
	int ret = -1;

	struct uprov_device *device = p_device;

	if (!block_device) {
		udo_log_error("Incorrect data passed\n");
		return NULL;
	}

	if (!device) {
		device = calloc(1, sizeof(struct uprov_device));
		if (!device) {
			udo_log_error("calloc: %s\n", strerror(errno));
			return NULL;
		}
	}

	strncpy(&(device->block_device[0]), \
		block_device, BLK_NAME_MAX - 1);

	ret = p_device_create_with_fdisk(device);
	if (ret == -1) {
		uprov_device_destroy(device);
		return NULL;
	}

	return device;
}

/*************************************
 * End uprov_device_create functions *
 *************************************/


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
