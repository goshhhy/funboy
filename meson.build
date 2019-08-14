project('kutaragi', 'c', default_options : ['buildtype=debugoptimized'])

sdl2 = dependency('SDL2')

executable('kutaragi-ps1', ['src/system/ps1/ps1.c', 'src/io/sdl.c', 'src/cpu/mips1/mips1.c', 
                        'src/device/generic_ram.c', 'src/device/generic_rom.c',
                         'src/device/generic_bus.c', 'src/device/generic_register.c',
                         'src/system/ps1/ps1.c',
                        ], dependencies : [sdl2], install : true)

executable('kutaragi-nes', ['src/io/sdl.c', 'src/cpu/mos6502/mos6502.c', 
                        'src/device/generic_ram.c', 'src/device/generic_rom.c',
                         'src/device/generic_bus.c', 'src/device/generic_register.c',
                         'src/system/nes/nes.c',
                        ], dependencies : [sdl2], install : true)

executable('kutaragi-gb', ['src/io/sdl.c', 'src/cpu/lr35902/lr35902.c', 
                        'src/device/generic_ram.c', 'src/device/generic_rom.c',
                         'src/device/generic_bus.c', 'src/device/generic_register.c',
                         'src/system/gb/gb.c',
                        ], dependencies : [sdl2], install : true)