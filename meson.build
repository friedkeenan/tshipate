project('tshipate', 'cpp',
    default_options : [
        'buildtype=release', # -O3
    ],
)

dependencies = []

dependencies += meson.get_compiler('cpp').find_library('pthread')

sources = []
subdir('source')

executable(meson.project_name(), sources,
    dependencies : dependencies,

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
