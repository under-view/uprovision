#ifndef UPROV_DEVICE_H
#define UPROV_DEVICE_H


enum uprov_device_type {
	UPROV_DEVICE = 0x00,
	UPROV_DEVICE_BLOCK_DEVICE = 0x01,
};


struct uprov_device_partition {
	long int number;
	long int startSector;
	long int endSector;
	long int sectorSize;
};


struct uprov_device {
	int                           blockDeviceFd;
	unsigned int                  blockSize;
	char                          *blockDevice;
	unsigned int                  partitionCount;
	struct uprov_device_partition *partitions;
	void                          *fdiskContext;
};


struct uprov_device_create_info {
	const char *blockDevice;
};


struct uprov_device *
uprov_device_create (struct uprov_device_create_info *device);


void
uprov_device_destroy (struct uprov_device *device);


struct uprov_device_resize_info {
	union {
		const char          *blockDevice;
		struct uprov_device *device;
	} resize;

	enum uprov_device_type deviceType;
	int                    partNum;
};


int
uprov_device_resize (struct uprov_device_resize_info *deviceModInfo);


#endif /* UPROV_DEVICE_H */
