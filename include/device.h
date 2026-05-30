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

#include <udo/macros.h>

/*
 * Stores information about the uprov_device context.
 */
struct uprov_device;


/*
 * @brief Given a block device parse with libfdisk and populate
 *        it's partitions in a struct uprov_device context.
 *
 * @param device       - May be NULL or a pointer to a struct uprov_device.
 *                       If NULL memory will be allocated and return to
 *                       caller. If not NULL address passed will be used
 *                       to store the newly created struct uprov_device
 *                       context.
 * @param block_device - Pointer to string storing name of block device
 *                       to associate with a struct uprov_device context.
 *
 * @returns
 *	on success: Pointer to a struct uprov_device
 *	on failure: NULL
 */
UDO_API
struct uprov_device *
uprov_device_create (struct uprov_device *device,
                     const char *block_device);


/*
 * @brief Frees any allocated memory and closes FD's (if open) create after
 *        uprov_device_create() call.
 *
 * @param device - Pointer to a valid struct uprov_device.
 */
UDO_API
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
