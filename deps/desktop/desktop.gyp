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
      'link_settings': {
        'libraries': [
          '-lz',
        ],
      },
      'conditions': [
        ['OS=="mac"', {
          'sources': [
            'src/DesktopOSX.m',
          ],
          'direct_dependent_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/Cocoa.framework',
              '$(SDKROOT)/System/Library/Frameworks/Webkit.framework',
            ],
          },
        }],
      ]
    },
  ]
}

