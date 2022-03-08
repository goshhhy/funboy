cmake_minimum_required(VERSION 3.00)

project(funboy C)
set(VERSION 0.1.0)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include_directories(src src/io src/cpu/sm83 src/system/gb src/device)

add_application(Funboy
    src/io/mac.c
	src/io/mac.r
	src/alarms.c
	src/cpu/sm83/sm83.c
	src/cpu/sm83/sm83_ops.c
	src/cpu/sm83/sm83_cached.c
	src/device/generic_ram.c
	src/device/generic_rom.c
	src/device/generic_bus.c
	src/device/generic_register.c
	src/system/gb/gb.c
	src/system/gb/serial_log.c
	src/system/gb/timer.c
	src/system/gb/dmg_ppu.c
	src/system/gb/input.c
	src/system/gb/mbc1.c
	src/system/gb/mbc3.c
    )
