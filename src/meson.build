libfdisk = dependency('fdisk', required: true)
libudo = dependency('udo', required: true)

fs = [ 'device.c' ]
libuprovision_deps = [libfdisk, libudo]

libuprovision_static = static_library(
  'uprov', files(fs),
  dependencies: libuprovision_deps,
  include_directories: inc,
  c_args: pargs
)
