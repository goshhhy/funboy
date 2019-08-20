#include <stdio.h>
#include <stdlib.h> 
#include <stdint.h>
#include <stdbool.h>

#include "device.h"

typedef struct ramInfo_s {
    size_t len;
    char* bytes;
} ramInfo_t;

static uint8_t GenericRamRead( busDevice_t *dev, uint32_t addr, bool final ) {
    ramInfo_t *ram;

    if ( !dev || !dev->data )
        return 0;
    ram = dev->data;

    if ( addr > ram->len ) {
        fprintf( stderr, "warning: GenericRamRead: address out of bounds ( access to %04x in a block  of size %04lx)\n", addr, ram->len );
        return 0;
    }
    return ram->bytes[addr];
}


static void GenericRamWrite( busDevice_t *dev, uint32_t addr, uint8_t val, bool final ) {
    ramInfo_t *ram;

    if ( !dev || !dev->data )
        return;
    ram = dev->data;

    if ( addr > ram->len ) {
        fprintf( stderr, "warning: GenericRamWrite: address out of bounds ( access to 0x%04x in a block  of size 0x%04lx)\n", addr, ram->len );
        return;
    }
    ram->bytes[addr] = val;
}

busDevice_t *GenericRam( size_t len ) {
    busDevice_t *dev;
    ramInfo_t *ram;

    if ( !( dev = malloc( sizeof( busDevice_t ) ) ) || !( ram = malloc( sizeof( ramInfo_t ) ) )) {
        fprintf( stderr, "couldn't allocate %lu byte ram block\n", len );
        return NULL;
    }
    ram->len = len;
    ram->bytes = calloc( 1, len );
    dev->data = ram;
    dev->Read8 = GenericRamRead;
    dev->Write8 = GenericRamWrite;
    printf( "created %lukb ram device\n", len / 1024 );
    return dev;
}
