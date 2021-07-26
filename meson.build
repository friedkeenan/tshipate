project('tshipate', 'cpp',
    default_options : [
        'buildtype=release', # -O3
    ],
)

sources = []
subdir('source')

executable(meson.project_name(), sources,
    link_args : '-s',
    cpp_args  : [
        '-std=gnu++20',

        '-Wall',
        '-Wextra',
        '-Werror',

        '-fno-exceptions',
        '-fno-rtti',
        '-fconcepts-diagnostics-depth=2',
    ]
)
