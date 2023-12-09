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
pname = meson.project_name()
so_version = '.'.join(meson.project_version())
cc = meson.get_compiler('c')

inc = include_directories('include')

subdir('include')
subdir('src')

libuprovision = library(
  pname,
  version: so_version,
  link_whole: libuprovision_static,
  include_directories: inc,
  c_args: pargs,
  install: true
)

if get_option('tests')
  subdir('tests')
endif

pkg = import('pkgconfig')

pkglibs = [libuprovision, libuprovision_deps]
pkgsubdirs = []
pkg.generate(libraries: pkglibs,
             subdirs : pkgsubdirs,
             version : so_version,
             name : pname,
             filebase : 'uprovision',
             description : 'Yet another disk manipulation libfdisk wrapper library & application')
