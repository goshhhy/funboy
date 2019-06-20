project('videospy', 'c', default_options : ['buildtype=debugoptimized'])

sdl2 = dependency('SDL2')
executable('videospy', ['src/main.c', 'src/io/sdl.c', 'src/cpu/mips1.c'], dependencies : [sdl2], install : true)