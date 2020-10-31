#include <stdio.h>
#include <stdlib.h> 

#include "device.h"

typedef struct ramInfo_s {
    size_t len;
    char* bytes;
    int disabled;
    unsigned char disabled_value;
} ramInfo_t;

static unsigned char GenericRamRead( busDevice_t *dev, unsigned long addr, int final ) {
    ramInfo_t *ram = dev->data;

    if ( addr >= ram->len ) {
        printf("warning: GenericRamRead: address out of bounds (access to %04lx in a block of size %04lx)\n", addr, ram->len );
    }

    if ( ram->disabled )
        return ram->disabled_value;
    else
        return ram->bytes[addr];
}


static void GenericRamWrite( busDevice_t *dev, unsigned long addr, unsigned char val, int final ) {
    ramInfo_t *ram = dev->data;
    
    ram->bytes[addr] = val;
}

busDevice_t *GenericRam( size_t len ) {
    busDevice_t *dev;
    ramInfo_t *ram;

    if ( !( dev = calloc( sizeof( busDevice_t ), 1 ) ) || !( ram = malloc( sizeof( ramInfo_t ) ) )) {
        fprintf( stderr, "couldn't allocate %lu byte ram block\n", len );
        return NULL;
    }
    ram->disabled = 0;
    ram->disabled_value = 0xff;
    ram->len = len;
    ram->bytes = calloc( len, 1 );
    dev->data = ram;
    dev->Read8 = GenericRamRead;
    dev->Write8 = GenericRamWrite;
    printf( "created %lukb ram device\n", len / 1024 );
    return dev;
}
