#ifndef UPROV_DEVICE_H
#define UPROV_DEVICE_H


enum uprov_device_type
{
	UPROV_DEVICE = 0x00,
	UPROV_DEVICE_BLOCK_DEVICE = 0x01,
};


struct uprov_device_create_info
{
	const char *block_device;
};


struct uprov_device *
uprov_device_create (struct uprov_device *device,
                     const void *device_info);


struct uprov_device_resize_info {
	union {
		const char          *block_device;
		struct uprov_device *device;
	} resize;

	enum uprov_device_type device_type;
	int                    part_num;
};


int
uprov_device_resize (struct uprov_device_resize_info *device_info);


void
uprov_device_destroy (struct uprov_device *device);

#endif /* UPROV_DEVICE_H */
