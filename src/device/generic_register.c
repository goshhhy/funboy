#include <stdio.h>
#include <stdlib.h> 

#include "device.h"

int printSteps = 0;

typedef struct regInfo_s {
    char* name;
    size_t len;
    unsigned char* data;
} regInfo_t;

static unsigned char GenericRegisterRead( busDevice_t *dev, busAddress_t addr, int final ) {
    regInfo_t *reg;

    if ( !dev || !dev->data )
        return 0;
    reg = dev->data;

    if ( addr > reg->len ) {
        fprintf( stderr, "warning: GenericRegisterRead: address out of bounds\n" );
        return 0;
    }
    if ( printSteps )
        printf( "read register [0x%08lx]%s:%08x\n", addr, reg->name, *reg->data  );
    if ( reg->data ) {
        return reg->data[addr];
    }
    return 0;
}


static void GenericRegisterWrite( busDevice_t *dev, busAddress_t addr, unsigned char val, int final ) {
    regInfo_t *reg;

    if ( !dev || !dev->data )
        return;
    reg = dev->data;

    if ( addr > reg->len ) {
        fprintf( stderr, "warning: GenericRegisterWrite: address out of bounds\n" );
        return;
    }

    if ( printSteps )
        fprintf( stdout, "write register [0x%08lx]%s <- %02x (byte %lu)\n", addr, reg->name, val, addr );
    if ( reg->data ) {
        reg->data[addr] = val;
    }
}

unsigned char *GenericRegisterdataPtr( busDevice_t *dev ) {
    regInfo_t *reg;

    if ( !dev || !dev->data )
        return NULL;
    reg = dev->data;

    return reg->data;
}

busDevice_t *GenericRegister( char *name, unsigned char *data, size_t len, unsigned char (*Read8)( struct busDevice_s* self, busAddress_t addr, int final ),
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
