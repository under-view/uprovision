# uprovision

Yet another disk manipulation libfdisk wrapper library and application implementation.
Used by underview to resize partitions.

**Building**
```
$ meson build
$ ninja -C build

# For Yocto SDK builds
$ meson setup --prefix="${SDKTARGETSYSROOT}/usr" \
              --libdir="${SDKTARGETSYSROOT}/usr/lib64" \
              build
$ ninja -C build
```

**Install**
```sh
$ ninja install -C build
```

**Inclusion**
```C
#include <uprovision/uprov.h>
```

**Testing**
```sh
$ ninja test -C build
```
