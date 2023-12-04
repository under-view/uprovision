project(
  'uprovision',
  'c',
  version: '0.0.1',
  license: 'MIT',
  meson_version: '>=0.56.0',
  default_options: [
    'warning_level=2',
    'werror=true',
    'prefix=/usr/'
  ],
)

pargs = []
so_version = meson.project_version()
cc = meson.get_compiler('c')

inc = include_directories('include')

subdir('src')

libuprovision = library(
  meson.project_name(),
  version: '.'.join(so_version),
  link_whole: libuprovision_static,
  include_directories: inc,
  c_args: pargs,
  install: true
)

if get_option('tests')
  subdir('tests')
endif
