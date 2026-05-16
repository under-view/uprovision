project(
  'uprovision',
  'c',
  version: '0.0.1',
  license: 'MIT',
  meson_version: '>=1.3.1',
  default_options: [
    'warning_level=2',
    'werror=true',
    'prefix=/usr/'
  ],
)

pargs = []
cc = meson.get_compiler('c')
project_name = meson.project_name()
so_version = meson.project_version()
src_root_dir = meson.current_source_dir()

inc = include_directories('include')

subdir('include')
subdir('src')

libuprovision = library(
  project_name, fs,
  version: so_version,
  include_directories: inc,
  dependencies: libuprovision_deps,
  c_args: pargs,
  install: true
)

libuprovision_a = static_library(
  project_name, fs,
  include_directories: inc,
  dependencies: libuprovision_deps,
  c_args: pargs,
  install: true
)

executable(project_name,
           'src/executables/uprovision.c',
           link_with: libuprovision,
           include_directories: inc,
           dependencies: [libudo],
           c_args: pargs,
           install: true)

if get_option('tests')
  subdir('tests')
endif

pkg = import('pkgconfig')

pkgsubdirs = []

pkglibs_shared = [
  libuprovision,
  libuprovision_deps
]

pkglibs_private = [
  libuprovision_a,
  libuprovision_deps
]

pkg.generate(libraries: pkglibs_shared,
             libraries_private: pkglibs_private,
             subdirs : pkgsubdirs,
             version : so_version,
             name : project_name,
             filebase : 'uprovision',
             description : 'Disk manipulation libfdisk wrapper library')
