project('quickjs', ['c','cpp'],
    version: '2021-03-27',
    license : 'MIT',
    default_options : [
        'c_std=c17',
        'cpp_std=c++latest',
        'cpp_eh=none',
        'cpp_rtti=false' 
    ]
)
add_project_arguments(
    [
       '-DJS_STRICT_NAN_BOXING',
       '-DCONFIG_BIGNUM'
    ],
    language : ['c','cpp']
)

### User option: JSX support
if get_option('jsx')==true
    add_project_arguments('-DCONFIG_JSX', language : ['c','cpp'])
endif

### User option: Storage support
if get_option('storage')==true
    add_project_arguments('-DCONFIG_STORAGE', language : ['c','cpp'])
    # TODO: cpp_eh=default needs to be set here, but not sure how (this will set /EHsc)
endif

### User option: Debugger support
if get_option('debugger')==true
    add_project_arguments('-DCONFIG_DEBUGGER', language : ['c','cpp'])
endif

# MSVC
if meson.get_compiler('c').get_id() == 'cl'
    add_project_arguments('-D_CRT_SECURE_NO_WARNINGS', language : ['c','cpp'])
endif

###
### QuickJS
###
quickjs_src = [
    'quickjs/cutils.h',
    'quickjs/cutils.c',
    'quickjs/libregexp.c',
    'quickjs/libunicode.c',
    'quickjs/quickjs.c',
    'quickjs/quickjs-libc.c',
    'quickjs/libbf.c',
    'quickjs/libregexp.h',
    'quickjs/libregexp-opcode.h',
    'quickjs/libunicode.h',
    'quickjs/libunicode-table.h',
    'quickjs/list.h',
    'quickjs/quickjs.h',
    'quickjs/quickjs-atom.h',
    'quickjs/quickjs-libc.h',
    'quickjs/quickjs-opcode.h',
]

if get_option('storage')==true
    quickjs_src += [
        'quickjs/storage/quickjs-storage.c',
        'quickjs/storage/dybase/src/btree.cpp',
        'quickjs/storage/dybase/src/database.cpp',
        'quickjs/storage/dybase/src/dybase.cpp',
        'quickjs/storage/dybase/src/padepool.cpp',
        'quickjs/storage/dybase/include/dybase.h',
    ]
endif

quickjs = library('quickjs', quickjs_src)

###
### QJSC
###
if get_option('qjsc')==true
    qjsc_src = [
        'quickjs/qjsc.c',
    ]
    qjsc = executable('qjsc', qjsc_src, link_with : quickjs, install : true, install_dir : meson.source_root() + '/build/bin')
endif

###
### QJS (please read notes)
###
if get_option('qjs')==true

    # TODO: This is not running as intended. What should happen is that qjsc from the
    #       previous compile step is invoked to generate c files from the two specified
    #       javascript files. These are then used in the build step of qjs
    #
    #       To build qjs, do the following:
    #           1) Configure quickjs with qjsc=true, then build
    #           2) Copy qjsc.exe to the /quickjs folder and run the following commands in a shell:
    #                   qjsc -c -o repl.c -m repl.js
    #                   qjsc -c -o qjscalc.c -m qjscalc.js
    #           3) Configure quickjs with qjs=true (and qjsc=false), then build 
    #
    #qjsc_prog = find_program('qjsc', required : true)
    #r = run_command(qjsc_prog, '-c', '-o', meson.source_root()+'/../quickjs/repl.c', '-m', meson.source_root()+'/../quickjs/repl.js', capture : true)
    #r = run_command(qjsc_prog, '-c', '-o', meson.source_root()+'/../quickjs/qjscalc.c', '-m', meson.source_root()+'/../quickjs/qjscalc.js', capture : true)

    qjs_src = [
        'quickjs/qjs.c',
        'quickjs/repl.c',
        'quickjs/qjscalc.c',
    ]
    qjsc = executable('qjs', qjs_src, link_with : quickjs)
endif
