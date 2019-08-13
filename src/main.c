#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "version.h"

#include "device/device.h"
#include "cpu/mips1/mips1.h"
#include "system/ps1/ps1.h"

int main( int argc, char **argv ) {
    printf( "kutaragi! v%u.%u.%u%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_DIST );

    busDevice_t *ps1bus = GenericBus( "ps1" );
    busDevice_t *iomap = Ps1IoMap();

    busDevice_t* ram = GenericRam( 2 * 1024 * 1024 );
    busDevice_t* rom = GenericRom( "psx.rom", 512 * 1024 );
    busDevice_t* scratchpad = GenericRam( 1 * 1024 );
    
    // KUSEG mappings
    GenericBusMapping( ps1bus, "ram.kuseg",     0x00000000, 0x001fffff, ram );
    GenericBusMapping( ps1bus, "ram.kuseg",     0x00200000, 0x003fffff, ram );
    GenericBusMapping( ps1bus, "ram.kuseg",     0x00400000, 0x005fffff, ram );
    GenericBusMapping( ps1bus, "ram.kuseg",     0x00600000, 0x007fffff, ram );
    GenericBusMapping( ps1bus, "rom.kuseg",     0x1fc00000, 0x1fc7ffff, rom );
    GenericBusMapping( ps1bus, "scratch.kuseg", 0x1f800000, 0x1f8003ff, scratchpad );
    GenericBusMapping( ps1bus, "iomap.kuseg",   0x1f801000, 0x1f802fff, iomap );
    // KSEG0 mappings
    GenericBusMapping( ps1bus, "ram.kseg0",     0x80000000, 0x801fffff, ram );
    GenericBusMapping( ps1bus, "ram.kseg0",     0x80200000, 0x803fffff, ram );
    GenericBusMapping( ps1bus, "ram.kseg0",     0x80400000, 0x805fffff, ram );
    GenericBusMapping( ps1bus, "ram.kseg0",     0x80600000, 0x807fffff, ram );
    GenericBusMapping( ps1bus, "rom.kseg0",     0x9fc00000, 0x9fc7ffff, rom );
    GenericBusMapping( ps1bus, "scratch.kseg0", 0x9f800000, 0x9f8003ff, scratchpad );
    GenericBusMapping( ps1bus, "iomap.kseg0",   0x9f801000, 0x9f802fff, iomap );
    // KSEG1 mappings
    GenericBusMapping( ps1bus, "ram.kseg1",     0xa0000000, 0xa01fffff, ram );
    GenericBusMapping( ps1bus, "ram.kseg1",     0xa0200000, 0xa03fffff, ram );
    GenericBusMapping( ps1bus, "ram.kseg1",     0xa0400000, 0xa05fffff, ram );
    GenericBusMapping( ps1bus, "ram.kseg1",     0xa0600000, 0xa07fffff, ram );
    GenericBusMapping( ps1bus, "rom.kseg1",     0xbfc00000, 0xbfc7ffff, rom );
    GenericBusMapping( ps1bus, "iomap.kseg1",   0xbf801000, 0xbf802fff, iomap );
    GenericBusMapping( ps1bus, "CacheControl",   0xfffe0130, 0xfffe0133,  GenericRegister( "CacheControl", NULL, 4, NULL, NULL ) );

    // KSEG2 mappings
    // GenericBusMapping( ps1bus, "cache.kseg2",   0xfffe0000, 0xbf801fff, cachecontrol );
    mips1_t *cpu = Mips1( ps1bus );

    char *bytes = GenericRomBytesPtr( rom );
    if ( bytes != NULL ) {
        char romVer[256], romCopyright[256];
        strncpy( romVer, &bytes[0x42e74], 255 );
        strncpy( romCopyright, &bytes[0x42e8c], 255 );
        printf( "\n%s%s\n", romVer, romCopyright );
    }

    while ( true ) {
	    cpu->Step( cpu );
    }
}
