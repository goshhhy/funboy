#include <stdio.h>
#include <stdint.h>

#include "version.h"
#include "busDevice.h"

int main( int argc, char **argv ) {
    printf( "kutaragi! v%u.%u.%u%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_DIST );

    
}