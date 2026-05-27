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


/*
 * @brief Returns size of the internal structure. So,
 *        if caller decides to allocate memory outside
 *        of API interface they know the exact amount
 *        of bytes.
 *
 * @returns
 *	on success: sizeof(struct uprov_device)
 *	on failure: sizeof(struct uprov_device)
 */
int
uprov_device_get_sizeof (void);

#endif /* UPROV_DEVICE_H */
