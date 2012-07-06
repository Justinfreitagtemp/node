{
  'targets': [
    {
      'target_name': 'desktop',
      'type': '<(library)',
      'include_dirs': [
        'src',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          'src',
        ],
        'cflags_cc': [
          '-x objective-c++',
        ],
        'cflags': [
          '-x objective-c++',
        ],
      },
      'sources': [
        'src/Desktop.h',
      ],
      'cflags': [
        '-O3',
        '-fomit-frame-pointer',
        '-malign-double',
        '-fstrict-aliasing',
        '-ffast-math',
        '-Wall',
      ],
      'conditions': [
        ['OS=="mac"', {
          'sources': [
            'src/DesktopOSX.mm',
          ],
          'direct_dependent_settings': {
            'libraries': [
              '-framework Cocoa',
              '-framework Webkit',
            ],
          },
          'xcode_settings': {
            'ARCHS': ['x86_64'],
          },
        }],
      ]
    },
  ]
}

