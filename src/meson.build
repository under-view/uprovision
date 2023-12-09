libfdisk = dependency('fdisk', required: true)
libhandy = dependency('handy', required: true)

fs = [ 'device.c' ]
libuprovision_deps = [libfdisk, libhandy]

libuprovision_static = static_library(
  'uprov', files(fs),
  dependencies: libuprovision_deps,
  include_directories: inc,
  c_args: pargs
)
