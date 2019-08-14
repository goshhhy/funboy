#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "../../version.h"
#include "../../device/device.h"
#include "../../cpu/lr35902/lr35902.h"

int main( int argc, char **argv ) {
    printf( "kutaragi!gb v%u.%u.%u%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_DIST );

    busDevice_t *gbbus = GenericBus( "gb" );

    busDevice_t* ram = GenericRam( 0x2000 );
    busDevice_t* zpage = GenericRam( 0x79 );
    busDevice_t* cram = GenericRam( 0x17ff );
    busDevice_t* bgram = GenericRam( 0x800 );
    busDevice_t* oam = GenericRam( 0x9f );
    busDevice_t* rom = GenericRom( argv[1], 32 * 1024 );
    
    GenericBusMapping( gbbus, "rom",     0x0000, 0x7fff, rom );
    GenericBusMapping( gbbus, "cram",    0x8000, 0x97ff, cram );
    GenericBusMapping( gbbus, "bgram",   0x9800, 0x9fff, bgram );
    GenericBusMapping( gbbus, "ram",     0xc000, 0xdfff, ram );
    GenericBusMapping( gbbus, "echo",    0xe000, 0xfdff, ram );
    GenericBusMapping( gbbus, "oam",     0xfe00, 0xfe9f, oam );
    GenericBusMapping( gbbus, "zpage",   0xff80, 0xfffe, zpage );

    lr35902_t *cpu = Lr35902( gbbus );

    while ( true ) {
	    cpu->Step( cpu );
    }
}