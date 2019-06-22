project('kutaragi', 'c', default_options : ['buildtype=debugoptimized'])

sdl2 = dependency('SDL2')
executable('kutaragi', ['src/main.c', 'src/io/sdl.c', 'src/cpu/mips1.c', 
                        'src/device/generic_ram.c', 'src/device/generic_rom.c', 'src/device/generic_bus.c',
                        ], dependencies : [sdl2], install : true)