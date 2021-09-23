project('funboy', 'c', default_options : ['buildtype=debugoptimized', 'c_std=c89'])

includes = include_directories(['src', 'src/cpu/sm83', 'src/device', 'src/io', 'src/system/gb'])

frontend = get_option('frontend')
if frontend == 'sdl'
    frontend_deps = dependency('sdl2')
    frontend_sources = ['src/io/sdl.c']
elif frontend == 'fbdev'
    frontend_deps = dependency('', required: false)
    frontend_sources = ['src/io/fbdev.c']
else
    error('invalid frontend target')
endif

bus_size = get_option('bus_size')
bus_impl_fast = get_option('bus_impl_fast')
bus_paranoid = get_option('bus_paranoid')

if bus_size == '32'
    add_project_arguments('-DBUS_SIZE_32', language : 'c')
else
    if bus_impl_fast == true
        add_project_arguments('-DBUS_IMPL_FAST16', language : 'c')
    endif
endif

if bus_paranoid == true
    add_project_arguments('-DBUS_PARANOID', language : 'c')
endif

sm83_trace = get_option('sm83_trace')

if sm83_trace == true
    add_project_arguments('-DSM83_TRACE', language : 'c')
endif

executable('funboy', [frontend_sources, 'src/cpu/sm83/sm83.c', 'src/cpu/sm83/sm83_ops.c', 
                        'src/cpu/sm83/sm83_cached.c', 'src/device/generic_ram.c', 
                        'src/device/generic_rom.c', 'src/device/generic_bus.c',
                        'src/device/generic_register.c', 'src/system/gb/gb.c',
                        'src/system/gb/serial_log.c', 'src/system/gb/timer.c',
                        'src/system/gb/dmg_ppu.c', 'src/system/gb/input.c',
                        'src/system/gb/mbc1.c', 'src/system/gb/mbc3.c',
                        'src/alarms.c',
                        ], dependencies : [frontend_deps], install : true, include_directories : includes)

