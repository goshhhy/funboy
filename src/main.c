#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "version.h"
#include "device/device.h"

int main( int argc, char **argv ) {
    printf( "kutaragi! v%u.%u.%u%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_DIST );

    busDevice_t *ps1bus = GenericBus( "ps1" );
    busDevice_t *iomap = GenericBus( "iomap" );

    busDevice_t* ram = GenericRam( 2 * 1024 * 1024 );
    busDevice_t* rom = GenericRom( "psx.rom", 512 * 1024 );
    busDevice_t* scratchpad = GenericRam( 1 * 1024 );
    
    // KUSEG mappings
    GenericBusMapping( ps1bus, "ram.kuseg",     0x00000000, 0x001fffff, ram );
    GenericBusMapping( ps1bus, "rom.kuseg",     0x1fc00000, 0x1fc7ffff, rom );
    GenericBusMapping( ps1bus, "scratch.kuseg", 0x1f800000, 0x1f8003ff, scratchpad );
    GenericBusMapping( ps1bus, "iomap.kuseg",   0x1f801000, 0x1f801fff, iomap );
    // KSEG0 mappings
    GenericBusMapping( ps1bus, "ram.kseg0",     0x80000000, 0x801fffff, ram );
    GenericBusMapping( ps1bus, "rom.kseg0",     0x9fc00000, 0x9fc7ffff, rom );
    GenericBusMapping( ps1bus, "scratch.kseg0", 0x9f800000, 0x9f8003ff, scratchpad );
    GenericBusMapping( ps1bus, "iomap.kseg0",   0x9f801000, 0x9f801fff, iomap );
    // KSEG1 mappings
    GenericBusMapping( ps1bus, "ram.kseg1",     0xa0000000, 0xa01fffff, ram );
    GenericBusMapping( ps1bus, "rom.kseg1",     0xbfc00000, 0xbfc7ffff, rom );
    GenericBusMapping( ps1bus, "iomap.kseg1",   0xbf801000, 0xbf801fff, iomap );
    // KSEG2 mappings
    // GenericBusMapping( ps1bus, "cache.kseg2",   0xfffe0000, 0xbf801fff, cachecontrol );
}
