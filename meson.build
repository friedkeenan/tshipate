project('tshipate', 'cpp',
    default_options : [
        'buildtype=release', # -O3
    ],
)

dependencies = []

dependencies += dependency('sfml-graphics')
dependencies += meson.get_compiler('cpp').find_library('pthread')

sources = []
subdir('source')

executable(meson.project_name(), sources,
    include_directories : 'include',
    dependencies        : dependencies,

    link_args : '-s',
    cpp_args  : [
        '-std=gnu++20',

        '-Wall',
        '-Wextra',
        '-Werror',

        '-fconcepts-diagnostics-depth=2',
    ]
)
