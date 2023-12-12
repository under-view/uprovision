#ifndef UPROV_DEVICE_H
#define UPROV_DEVICE_H


struct uprov_device_partitions {
	long int number;
	long int startSector;
	long int endSector;
	long int sectorSize;
};


struct uprov_device {
	unsigned int                   blockSize;
	char                           *blockDevice;
	unsigned int                   partitionCount;
	struct uprov_device_partitions *partitions;
};


struct uprov_device_create_info {
	char *blockDevice;
};


struct uprov_device *
uprov_device_create (struct uprov_device_create_info *device);


void
uprov_device_destroy (struct uprov_device *device);

#endif /* UPROV_DEVICE_H */
