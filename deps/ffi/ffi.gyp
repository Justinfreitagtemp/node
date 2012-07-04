{
  'targets': [
    {
      'target_name': 'ffi',
      'type': '<(library)',
      'include_dirs': [
        'include',
        'src',
        'src/x86',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          'include',
          'src/x86',
        ],
      },
      'defines': [ 'USE_DL_PREFIX' ],
      'sources': [
        'include/ffi.h',
        'include/ffi_common.h',
        'include/fficonfig.h',
        'src/closures.c',
        'src/debug.c',
        'src/dlmalloc.c',
        'src/java_raw_api.c',
        'src/prep_cif.c',
        'src/raw_api.c',
        'src/types.c',
        'src/x86/ffitarget.h',
        'src/x86/ffi64.c',
        'src/x86/ffi.c',
      ],
      'cflags': [
        '-g',
        '--std=gnu89',
        '-pedantic',
        '-Wall',
        '-Wextra',
        '-Wno-unused-parameter'
      ],
      'conditions': [
        [ 'OS=="win"', {
          'sources': [
            'src/x86/win32.S',
            'src/x86/win64.S',
          ]
        }],
        [ 'OS=="mac"', {
          'sources': [
            'src/x86/darwin.S',
            'src/x86/darwin64.S',
          ]
        }],
        [ 'OS=="linux"', {
          'sources': [
            'src/x86/unix64.S',
          ]
        }],
        [ 'OS=="freebsd"', {
          'sources': [
            'src/x86/freebsd.S',
          ]
        }],
      ],
      'link_settings': {
        'libraries': [
          '-lz',
        ],
      },
    },
  ]
}

