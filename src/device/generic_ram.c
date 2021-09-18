#include <stdio.h>
#include <stdlib.h> 

#include "device.h"

static unsigned char GenericRamRead( busDevice_t *dev, busAddress_t addr, int final ) {
    ramInfo_t *ram = dev->data;

#ifdef BUS_MAP_PARANOID
    if ( addr >= ram->len ) {
        printf("%s: warning: GenericRamRead: address out of bounds (access to %04x in a block of size %04lx)\n", ram->name, addr, ram->len );
    }
#endif

    if ( ram->disabled )
        return ram->disabled_value;
    
    return ram->bytes[addr];
}


static void GenericRamWrite( busDevice_t *dev, busAddress_t addr, unsigned char val, int final ) {
    ramInfo_t *ram = dev->data;
    
#ifdef BUS_MAP_PARANOID
    if ( addr >= ram->len ) {
        printf("%s: warning: GenericRamWrite: address out of bounds (access to %04x in a block of size %04lx)\n", ram->name, addr, ram->len );
    }
#endif

    if ( ram->disabled )
        return;

    ram->bytes[addr] = val;
}

static void * GenericRamDataPtr( busDevice_t *dev ) {
    ramInfo_t *ram = dev->data;
    
    return ram->bytes;
}

busDevice_t *GenericRam( size_t len, char * name ) {
    busDevice_t *dev;
    ramInfo_t *ram;

    if ( !( dev = calloc( sizeof( busDevice_t ), 1 ) ) || !( ram = malloc( sizeof( ramInfo_t ) ) )) {
        fprintf( stderr, "couldn't allocate %lu byte ram block\n", len );
        return NULL;
    }
    ram->name = name;
    ram->disabled = 0;
    ram->disabled_value = 0xff;
    ram->len = len;
    ram->bytes = calloc( len, 1 );
    dev->data = ram;
    dev->Read8 = GenericRamRead;
    dev->Write8 = GenericRamWrite;
    dev->DataPtr = GenericRamDataPtr;
    printf( "created %lukb ram device\n", len / 1024 );
    return dev;
}
