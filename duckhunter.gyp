{
    'include': [
        'common.gypi',
    ],
    'targets': [
        {
            'target_name': 'duckhunter',
            'product_name': 'duckhunter',
            'type': 'executable',
            'cflags': [
                '-pedantic',
                '-Wall',
                '-std=c11',
                '-flto',
                '-g',
                '-O3',
            ],
            'sources': [
                'src/connector.c',
                'src/main.c',
                'src/process.c',
            ],
            'include_dirs': [
                'include',
                'external/libuv/include',
                'external/libzmq/include',
                'external/bstring/bstring',
                'external/glib',
                'external/glib/glib',
            ],
            'dependencies': [
                'external/libuv/uv.gyp:libuv',
            ],
            'link_settings': {
                'libraries': [
                    '-lzmq',
                    '-lbstring',
                    '-lglib-2.0',
                ],
                'ldflags': [
                    '-L../../external/libzmq/src/.libs',
                    '-L../../external/bstring/bstring/.libs',
                    '-L../../external/glib/glib/.libs',
                    '-flto',
                ],
            },
        },
    ]
}
