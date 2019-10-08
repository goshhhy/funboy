#include <stdio.h>
#include <stdlib.h> 
#include <stdint.h>
#include <stdbool.h>

#include "device.h"

typedef struct regInfo_s {
    char* name;
    size_t len;
    uint8_t* data;
} regInfo_t;

static uint8_t GenericRegisterRead( busDevice_t *dev, uint32_t addr, bool final ) {
    regInfo_t *reg;

    if ( !dev || !dev->data )
        return 0;
    reg = dev->data;

    if ( addr > reg->len ) {
        fprintf( stderr, "warning: GenericRegisterRead: address out of bounds\n" );
        return 0;
    }
    printf( "read register [0x%08x]%s:%08x\n", addr, reg->name, *reg->data  );
    if ( reg->data ) {
        return reg->data[addr];
    }
    return 0;
}


static void GenericRegisterWrite( busDevice_t *dev, uint32_t addr, uint8_t val, bool final ) {
    regInfo_t *reg;

    if ( !dev || !dev->data )
        return;
    reg = dev->data;

    if ( addr > reg->len ) {
        fprintf( stderr, "warning: GenericRegisterWrite: address out of bounds\n" );
        return;
    }

    printf( "write register [0x%08x]%s <- %02x (byte %u)\n", addr, reg->name, val, addr );
    if ( reg->data ) {
        reg->data[addr] = val;
    }
}

uint8_t *GenericRegisterdataPtr( busDevice_t *dev ) {
    regInfo_t *reg;

    if ( !dev || !dev->data )
        return NULL;
    reg = dev->data;

    return reg->data;
}

busDevice_t *GenericRegister( char *name, uint8_t *data, size_t len, uint8_t (*Read8)( struct busDevice_s* self, uint32_t addr, bool final ),
                                    void (*Write8)( struct busDevice_s* self, uint32_t addr, uint8_t val, bool final ) ) {
    busDevice_t *dev;
    regInfo_t *reg;

    if ( !( dev = malloc( sizeof( busDevice_t ) ) ) || !( reg = malloc( sizeof( regInfo_t ) ) ) ) {
        fprintf( stderr, "couldn't allocate ram\n" );
        return NULL;
    }
    if ( !data ) {
        data = malloc( sizeof( uint8_t ) * len );
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
