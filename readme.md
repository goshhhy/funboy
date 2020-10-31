# funboy

this is a gameboy emulator which currently only emulates the original gameboy.

it is designed to be relatively simple while also making efficient use of cpu time,
allowing it to run on low-end processors - think double-digit MHz.

it uses an interpreter with two modes of operation. standard mode has minimal memory
overhead, but uses more cpu time as it has to decode on every execution step. cached
mode stores the decode results allowing much faster execution, but requires more
memory - at least half a megabyte, and up to several megabytes, depending on the
exact configuration.

this emulator is imperfect. the cpu and timing emulation is relatively accurate, but
the ppu emulation needs much work, and sound emulation is currently missing entirely.