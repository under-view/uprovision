libcmocka = dependency('cmocka', required: true)

progs = [ 'test-device.c' ]

original_args = pargs
foreach p : progs
  exec_name = p.substring(0,-2) # remove .c extension from name

  exec = executable(exec_name, p,
                    link_with: libuprovision,
                    dependencies: [libcmocka],
                    include_directories: [inc],
                    c_args: pargs,
                    install: false)

  test(exec_name, exec)
  pargs = original_args
endforeach
