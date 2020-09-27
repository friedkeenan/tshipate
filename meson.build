project('tshipate', 'cpp',
    default_options : 'buildtype=debugoptimized',
)

dependencies = []
dependencies += dependency('sfml-all')
dependencies += meson.get_compiler('cpp').find_library('pthread')

sources = []
subdir('source')

executable(meson.project_name(), sources,
    dependencies : dependencies,

    link_args : '-s',
    cpp_args  : [
        '-std=gnu++20',

        '-fno-exceptions',
        '-fno-rtti',
        '-fconcepts-diagnostics-depth=2',

        '-Wall',
        '-Wextra',
        '-Werror',

        '-Wno-unused-parameter',
    ]
)
