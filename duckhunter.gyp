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
            ],
            'dependencies': [
                'external/libuv/uv.gyp:libuv',
            ],
            'link_settings': {
                'libraries': [
                    '-lzmq',                    
                ],
                'ldflags': [
                    '-L../../external/libzmq/src/.libs'
                ],
            },
        },
    ]
}
