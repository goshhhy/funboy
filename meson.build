project('funboy', 'c', default_options : ['buildtype=debugoptimized'])

sdl2 = dependency('SDL2')

executable('funboy', ['src/io/sdl.c', 'src/cpu/sm83/sm83.c', 
                        'src/device/generic_ram.c', 'src/device/generic_rom.c',
                         'src/device/generic_bus.c', 'src/device/generic_register.c',
                         'src/system/gb/gb.c', 'src/system/gb/serial_log.c',
                         'src/system/gb/timer.c', 'src/system/gb/dmg_ppu.c',
                         'src/system/gb/input.c',
                        ], dependencies : [sdl2], install : true)
