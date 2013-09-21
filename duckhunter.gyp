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
            ],
            'sources': [
                'src/connector.c',
                'src/main.c',
            ],
            'include_dirs': [
                'include',
                'external/libuv/include',
                'external/libzmq/include',
                'external/bstring/bstring',
            ],
            'dependencies': [
                'external/libuv/uv.gyp:libuv',
            ],
            'link_settings': {
                'libraries': [
                    '-lzmq',
                    '-lbstring',
                ],
                'ldflags': [
                    '-L../../external/libzmq/src/.libs',
                    '-L../../external/bstring/bstring/.libs'
                ],
            },
        },
    ]
}
