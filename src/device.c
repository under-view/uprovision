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
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <linux/fs.h>

#include <libfdisk/libfdisk.h>
#include <udo/udo.h>

#include "device.h"

#define IS_GPT            0x4B
#define PARTLABEL_MAX     72
#define BLK_NAME_MAX      (1<<5)
#define TABLE_TYPE_MAX    (1<<3)
#define PARTITIONS_MAX    (1<<7)
#define FSTYPE_MAX        (1<<8)
#define TYPE_CODE_STR_MAX (1<<6)
#define PART_NAME_MAX     (BLK_NAME_MAX+3)

/*
 * @brief Structure defining a single partition entry.
 *        For a caller given partitioned block device.
 *
 * @member number        - Partition number.
 * @member start_sector  - The starting sector of a partition.
 * @member end_sector    - The final sector of a partition.
 * @member sector_size   - Amount of sectors a partition has.
 * @member logical       - Boolean indicating if partition
 *                         logical or not. Used for MBR.
 *                         Always false in the GPT case.
 * @member extended      - Boolean indicating if partition
 *                         extended or not. Used for MBR.
 *                         Always false in the GPT case.
 * @member fstype        - File system type of a partition.
 * @member partlabel     - GPT partition label of a partition.
 * @member fslabel       - File system label of a partition.
 * @member type          - Stores the partition type.
 *      @member code     - Stores partition type
 *                         (Linux, Linux extended, etc..)
 *                         in integer representation.
 *                         Used MBR based partition table types.
 *      @member code_str - Store partition type in string format.
 *                         GUID partition table type in string format.
 *                         Used GPT based partition table types.
 *                         https://wiki.archlinux.org/title/GPT_fdisk#Partition_type
 */
struct uprov_device_part
{
	size_t   number;
	uint64_t start_sector;
	uint64_t end_sector;
	uint64_t sector_size;
	bool     logical;
	bool     extended;
	char     fstype[FSTYPE_MAX];
	char     fslabel[FSLABEL_MAX];
	char     partlabel[PARTLABEL_MAX];
	union _type_code {
		uint32_t code;
		char     code_str[TYPE_CODE_STR_MAX];
	} type;
};


/*
 * @brief Structure storing everything required
 *        to repartition a block device.
 *
 * @member err          - Stores information about the error that occured
 *                        for the given context and may later be retrieved
 *                        by caller.
 * @member free         - If structure allocated with calloc(3) member will be
 *                        set to true so that, we know to call free(3) when
 *                        destroying the context.
 * @member parts        - Array of partitions for the given @block_device.
 * @member block_device - Block device name in string format.
 * @member table_type   - Partition table type used by @block_device.
 * @member part_name    - Used to temporarily acquire and store name
 *                        of a partition. Or the absolute path to
 *                        the block device partition.
 * @member fd           - @block_device open file descriptor.
 * @member sector_sz    - Byte size of each sector of a block device.
 *                        Typically 512 bytes per sector.
 */
struct uprov_device
{
	struct udo_log_error_struct err;
	bool                        free;
	struct uprov_device_part    parts[PARTITIONS_MAX];
	char                        block_device[BLK_NAME_MAX];
	char                        table_type[TABLE_TYPE_MAX];
	char                        part_name[PART_NAME_MAX];
	int                         fd;
	uint32_t                    sector_sz;
	size_t                      part_count;
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

static int
p_device_create_with_fdisk (struct uprov_device *device)
{
	uint32_t p;
	uint8_t gpt;
	int err = -1;

	struct p_uprov_fdisk fdisk;

	struct fdisk_label *lb = NULL;
	struct fdisk_partition *part = NULL;
	struct fdisk_parttype  *parttype = NULL;

	char *fstype = NULL, *fslabel = NULL, *partlabel = NULL;

	memset(&fdisk, 0, sizeof(struct p_uprov_fdisk));

	fdisk.ctx = fdisk_new_context();
	if (!(fdisk.ctx)) {
		udo_log_error("fdisk_new_context failed\n");
		return -1;
	}

	device->fd = open(device->block_device, O_RDWR);
	if (device->fd == -1) {
		udo_log_error("open: %s\n", strerror(errno));
		p_uprov_fdisk_destroy(&fdisk);
		return -1;
	}

	err = fdisk_assign_device_by_fd(fdisk.ctx, \
		device->fd, device->block_device, 0);
	if (err < 0) {
		udo_log_error("fdisk_assign_device_by_fd('%d','%s') failed\n",
		              device->fd, device->block_device);
		p_uprov_fdisk_destroy(&fdisk);
		return -1;
	}

	err = fdisk_get_partitions(fdisk.ctx, &(fdisk.table));
	if (err != 0) {
		udo_log_error("fdisk_get_partitions('%d','%s') failed\n",
		              device->fd, device->block_device);
		p_uprov_fdisk_destroy(&fdisk);
		return -1;
	}

	device->sector_sz = fdisk_get_sector_size(fdisk.ctx);
	device->part_count = fdisk_table_get_nents(fdisk.table);

	lb = fdisk_get_label(fdisk.ctx, NULL);
	strncpy(device->table_type, fdisk_label_get_name(lb), TABLE_TYPE_MAX-1);
	gpt = UDO_STRTOU(device->table_type);

	for (p = 0; p < device->part_count; p++) {
		part = fdisk_table_get_partition_by_partno(fdisk.table, p);

		device->parts[p].number = fdisk_partition_get_partno(part);
		device->parts[p].start_sector = fdisk_partition_get_start(part);
		device->parts[p].end_sector = fdisk_partition_get_end(part);
		device->parts[p].sector_size = fdisk_partition_get_size(part);

		/* Acquire fslabel of a partiton */
		fdisk_partition_to_string(part, fdisk.ctx,
					  FDISK_FIELD_FSLABEL,
					  &fslabel);
		strncpy(device->parts[p].fslabel, \
			(fslabel) ? fslabel : \
			&(char){'\0'}, FSLABEL_MAX);
		free(fslabel); fslabel = NULL;

		/* Acquire fstype of a partition */
		fdisk_partition_to_string(part, fdisk.ctx,
		                          FDISK_FIELD_FSTYPE,
		                          &fstype);
		strncpy(device->parts[p].fstype, \
		        (fstype) ? fstype : \
			&(char){'\0'}, FSTYPE_MAX);
		free(fstype); fstype = NULL;

		parttype = fdisk_partition_get_type(part);

		if (gpt == IS_GPT) {
			strncpy(device->parts[p].type.code_str,
				fdisk_parttype_get_string(parttype),
				TYPE_CODE_STR_MAX);

			fdisk_partition_to_string(part, fdisk.ctx,
						  FDISK_FIELD_NAME,
						  &partlabel);
			strncpy(device->parts[p].partlabel,
				(partlabel) ? partlabel : \
				&(char){'\0'}, PARTLABEL_MAX);
			free(partlabel); partlabel = NULL;
		} else {
			device->parts[p].type.code = \
				fdisk_parttype_get_code(parttype);

			if (fdisk_partition_is_nested(part)) {
				device->parts[p].logical = true;
			} else if (fdisk_partition_is_container(part)) {
				device->parts[p].extended = true;
			}
		}

		fdisk_unref_parttype(parttype); parttype = NULL;
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

	close(device->fd);

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
