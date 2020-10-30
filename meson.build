project('funboy', 'c', default_options : ['buildtype=debugoptimized', 'c_std=c89'])

sdl2 = dependency('SDL2')

executable('funboy', ['src/io/sdl.c', 'src/cpu/sm83/sm83.c', 'src/cpu/sm83/sm83_ops.c',
                        'src/device/generic_ram.c', 'src/device/generic_rom.c',
                         'src/device/generic_bus.c', 'src/device/generic_register.c',
                         'src/system/gb/gb.c', 'src/system/gb/serial_log.c',
                         'src/system/gb/timer.c', 'src/system/gb/dmg_ppu.c',
                         'src/system/gb/input.c', 'src/system/gb/mbc1.c',
                         'src/system/gb/mbc3.c',
                        ], dependencies : [sdl2], install : true)

