#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>


#include "../../version.h"
#include "../../device/device.h"
#include "../../cpu/mos6502/mos6502.h"

int main( int argc, char **argv ) {
    printf( "kutaragi!nes v%u.%u.%u%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_DIST );

    busDevice_t *nesbus = GenericBus( "nes" );
    busDevice_t* ram = GenericRam( 2 * 1024 );
}