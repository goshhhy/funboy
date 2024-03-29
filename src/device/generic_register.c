#include <stdio.h>
#include <stdlib.h> 

#include "device.h"

/* #define BUS_REGISTER_VERBOSE */

static unsigned char GenericRegisterRead( busDevice_t *dev, busAddress_t addr, int final ) {
    regInfo_t *reg;

    #ifdef BUS_MAP_PARANOID
    if ( !dev || !dev->data )
        return 0;
    #endif

    reg = dev->data;

    #ifdef BUS_MAP_PARANOID
    if ( addr > reg->len ) {
        fprintf( stderr, "warning: GenericRegisterRead: address out of bounds\n" );
        return 0;
    }
    #endif

    #ifdef BUS_REGISTER_VERBOSE
    printf( "read register [0x%08lx]%s:%08x\n", addr, reg->name, *reg->data  );
    #endif

    if ( reg->data ) {
        return ((unsigned char*)reg->data)[addr];
    }
    return 0;
}

static void GenericRegisterWrite( busDevice_t *dev, busAddress_t addr, unsigned char val, int final ) {
    regInfo_t *reg;

    #ifdef BUS_MAP_PARANOID
    if ( !dev || !dev->data )
        return;
    #endif

    reg = dev->data;

    #ifdef BUS_MAP_PARANOID
    if ( addr > reg->len ) {
        fprintf( stderr, "warning: GenericRegisterWrite: address out of bounds\n" );
        return;
    }
    #endif

    #ifdef BUS_REGISTER_VERBOSE
    fprintf( stdout, "write register [0x%08lx]%s <- %02x (byte %lu)\n", addr, reg->name, val, addr );
    #endif

    if ( reg->data ) {
        ((unsigned char*)reg->data)[addr] = val;
    }
}

void GenericRegisterReadOnly( busDevice_t *dev, busAddress_t addr, unsigned char val, int final ) {
    printf("write to read-only register\n");
}

unsigned char *GenericRegisterdataPtr( busDevice_t *dev ) {
    regInfo_t *reg;

    if ( !dev || !dev->data )
        return NULL;
    reg = dev->data;

    return reg->data;
}

busDevice_t *GenericRegister( char *name, void *data, size_t len, unsigned char (*Read8)( struct busDevice_s* self, busAddress_t addr, int final ),
                                    void (*Write8)( struct busDevice_s* self, busAddress_t addr, unsigned char val, int final ) ) {
    busDevice_t *dev;
    regInfo_t *reg;

    if ( !( dev = malloc( sizeof( busDevice_t ) ) ) || !( reg = malloc( sizeof( regInfo_t ) ) ) ) {
        fprintf( stderr, "couldn't allocate ram\n" );
        return NULL;
    }
    if ( !data ) {
        data = calloc( sizeof( unsigned char ) * len, 1 );
    }
    reg->name = name;
    reg->len = len;
    reg->data = data;
    dev->data = reg;

    if ( Read8 != NULL )
        dev->Read8 = Read8;
    else
        dev->Read8 = GenericRegisterRead;

    if ( Write8 != NULL )
        dev->Write8 = Write8;
    else
        dev->Write8 = GenericRegisterWrite;

    printf( "created reg device %s\n", name);
    return dev;
}
