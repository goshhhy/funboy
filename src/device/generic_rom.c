#include <stdio.h>
#include <stdlib.h> 

#include "device.h"

typedef struct romInfo_s {
    char* name;
    size_t len;
    char* bytes;
} romInfo_t;

static unsigned char GenericRomRead( busDevice_t *dev, busAddress_t addr, int final ) {
    romInfo_t *rom;

    #ifdef BUS_MAP_PARANOID
    if ( !dev || !dev->data )
        return 0;
    #endif

    rom = dev->data;

    if ( addr > rom->len ) {
        fprintf( stderr, "warning: GenericRomRead: address out of bounds [%s:%04x]\n", rom->name, addr );
        return 0;
    }
    return rom->bytes[addr];
}


static void GenericRomWrite( busDevice_t *dev, busAddress_t addr, unsigned char val, int final ) {
    romInfo_t *rom;

    #ifdef BUS_MAP_PARANOID
    if ( !dev || !dev->data )
        return;
    #endif

    rom = dev->data;

    if ( addr > rom->len ) {
        fprintf( stderr, "warning: GenericRomWrite: address out of bounds\n" );
        exit(1);
        return;
    }
    fprintf( stderr, "warning: discarded write to rom area [0x%04x]\n", addr );
    /* exit(1); */
}

char* GenericRomBytesPtr( busDevice_t *dev ) {
    romInfo_t *rom;

    #ifdef BUS_MAP_PARANOID
    if ( !dev || !dev->data )
        return NULL;
    #endif

    rom = dev->data;

    return rom->bytes;
}

busDevice_t *GenericRom( char *fileName, size_t len ) {
    FILE *f = fopen( fileName, "r" );
    busDevice_t *dev;
    romInfo_t *rom;
    size_t rem;

    if ( !f ) {
        fprintf( stderr, "couldn't open rom file \"%s\"\n", fileName );
        return NULL;
    }
    if ( !( dev = malloc( sizeof( busDevice_t ) ) ) || !( rom = malloc( sizeof( romInfo_t ) ) ) ) {
        fprintf( stderr, "couldn't allocate ram for %s\n", fileName );
        fclose( f );
        return NULL;
    }
    rom->name = fileName;
    rom->len = len;
    rom->bytes = calloc( 1, len );
    dev->data = rom;
    dev->Read8 = GenericRomRead;
    dev->Write8 = GenericRomWrite;
    for ( rem = len; rem > 0; ) {
        size_t read = fread( rom->bytes, 1, rem, f );
        rem = rem - read;
        if ( read == 0 ) {
            if ( feof( f ) ) {
            fprintf( stderr, "warning: GenericRom: hit eof with %lu bytes remaining while loading %s\n"\
                     "                     (check if file is correct)\n", rem, fileName );
            } else if ( ferror( f ) ) {
                fprintf( stderr, "warning: GenericRom: hit error with %lu bytes remaining while loading %s\n"\
                     "                     (check if file is correct)\n", rem, fileName );
            }
            break;
        }
    }
    printf( "created rom device from %s\n", fileName );
    fclose( f );
    return dev;
}
