libcmocka = dependency('cmocka', required: true)
block_device = get_option('block-device')

progs = [ 'test-device.c' ]

original_args = pargs
foreach p : progs
  exec_name = p.substring(0,-2) # remove .c extension from name

  if p == 'test-device.c'
    pargs += [
      '-DBLOCK_DEVICE="' + block_device + '"',
    ]
  endif

  exec = executable(exec_name, p,
                    link_with: libuprovision,
                    dependencies: [libcmocka, libcando],
                    include_directories: [inc],
                    c_args: pargs,
                    install: false)

  test(exec_name, exec)
  pargs = original_args
endforeach
