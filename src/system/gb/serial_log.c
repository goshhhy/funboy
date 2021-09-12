#include <stdio.h>
#include <string.h>

#include "version.h"
#include "device.h"
#include "sm83.h"

typedef struct regInfo_s {
    char* name;
    size_t len;
    unsigned long* data;
} regInfo_t;

unsigned char datReg = 0;

static unsigned char SerialRegisterRead( busDevice_t *dev, busAddress_t addr, int final ) {
    regInfo_t *reg;

    if ( !dev || !dev->data )
        return 0;
    reg = dev->data;

    if ( addr > reg->len ) {
        fprintf( stderr, "warning: SerialRegisterRead: address out of bounds\n" );
        return 0;
    }
    /* printf( "read register [0x%08x]%s:%08x\n", addr, reg->name, *reg->data  ); */
    if ( reg->data ) {
        return reg->data[addr];
    }
    return 0;
}

static void SerialRegisterWrite( busDevice_t *dev, busAddress_t addr, unsigned char val, int final ) {
    regInfo_t *reg;

    if ( !dev || !dev->data )
        return;
    reg = dev->data;

    if ( addr > reg->len ) {
        fprintf( stderr, "warning: SerialRegisterWrite: address out of bounds\n" );
        return;
    }

    /* printf( "write register [0x%08x]%s <- %02x (byte %u)\n", addr, reg->name, val, addr ); */
    if ( reg->data ) {
        reg->data[addr] = val;
    }
    if ( val == 0x81 ) {
        fprintf( stderr, "%c", datReg );
    }
}

void SerialToStderr( busDevice_t* bus ) {
    GenericBusMapping( bus, "SerDat", 0xFF01, 0xFF01, GenericRegister( "SerDat", &datReg, 1, NULL, NULL ) );
    GenericBusMapping( bus, "SerTxCtl", 0xFF02, 0xFF02, GenericRegister( "SerTxCtl", NULL, 1, SerialRegisterRead, SerialRegisterWrite ) );
}

