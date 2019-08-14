#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "../../version.h"
#include "../../device/device.h"
#include "../../cpu/mos6502/mos6502.h"

#include "vic20.h"

int main( int argc, char **argv ) {
    bool go = true;
    printf( "kutaragi!vic20 v%u.%u.%u%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_DIST );

    busDevice_t *vicbus = GenericBus( "vic" );
    busDevice_t* mainram = GenericRam( 0x1000 );
    busDevice_t* lowram = GenericRam( 0x400 );
    busDevice_t* charRom = GenericRom( "char.rom", 0x1000 );
    busDevice_t* basicRom = GenericRom( "basic.rom", 0x2000 );
    busDevice_t* kernalRom = GenericRom( "kernal.rom", 0x2000 );
    busDevice_t* vicvideo = VicVideo();

    GenericBusMapping( vicbus, "lowram", 0x0000, 0x03ff, lowram );
    GenericBusMapping( vicbus, "mainram", 0x1000, 0x1fff, mainram );
    GenericBusMapping( vicbus, "charRom", 0x8000, 0x8fff, charRom );
    GenericBusMapping( vicbus, "vid", 0x9000, 0x9010, vicvideo );
    GenericBusMapping( vicbus, "basicRom", 0xc000, 0xdfff, basicRom );
    GenericBusMapping( vicbus, "kernalRom", 0xe000, 0xffff, kernalRom );

    GenericBusSetEmptyVal( vicbus, 0x00 );

    mos6502_t *cpu = Mos6502( vicbus );

    while( go ) {
        cpu->Step( cpu );
        if ( Vic_Step( vicbus ) ) {
            go = IO_Update();
        }
    }
}