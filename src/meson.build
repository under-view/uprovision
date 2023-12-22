libfdisk = dependency('fdisk', required: true)
libcando = dependency('cando', required: true)

fs = [ 'device.c' ]
libuprovision_deps = [libfdisk, libcando]

libuprovision_static = static_library(
  'uprov', files(fs),
  dependencies: libuprovision_deps,
  include_directories: inc,
  c_args: pargs
)
