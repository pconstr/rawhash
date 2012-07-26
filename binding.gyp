{
  "targets": [
    {
      "target_name": "rawhash",
      "sources": [
        "src/rawhash.cpp",
        "src/MurmurHash3.h",
        "src/MurmurHash3.cpp"
      ],
      'cflags': [ '<!@(pkg-config --cflags libsparsehash)' ],
      'conditions': [
        [ 'OS=="linux" or OS=="freebsd" or OS=="openbsd" or OS=="solaris"', {
          'cflags_cc!': ['-fno-rtti', '-fno-exceptions'],
          'cflags_cc+': ['-frtti', '-fexceptions'],
        }],
        ['OS=="mac"', {
          'xcode_settings': {
            'GCC_ENABLE_CPP_RTTI': 'YES',
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
          }
        }]
      ]
    }
  ]
}