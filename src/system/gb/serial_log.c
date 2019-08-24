#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "../../version.h"
#include "../../device/device.h"
#include "../../cpu/sm83/sm83.h"

typedef struct regInfo_s {
    char* name;
    size_t len;
    uint32_t* data;
} regInfo_t;

uint8_t datReg;

static uint8_t SerialRegisterRead( busDevice_t *dev, uint32_t addr, bool final ) {
    regInfo_t *reg;

    if ( !dev || !dev->data )
        return 0;
    reg = dev->data;

    if ( addr > reg->len ) {
        fprintf( stderr, "warning: SerialRegisterRead: address out of bounds\n" );
        return 0;
    }
    printf( "read register [0x%08x]%s:%08x\n", addr, reg->name, *reg->data  );
    if ( reg->data ) {
        return reg->data[addr];
    }
}

static void SerialRegisterWrite( busDevice_t *dev, uint32_t addr, uint8_t val, bool final ) {
    regInfo_t *reg;

    if ( !dev || !dev->data )
        return;
    reg = dev->data;

    if ( addr > reg->len ) {
        fprintf( stderr, "warning: SerialRegisterWrite: address out of bounds\n" );
        return;
    }

    printf( "write register [0x%08x]%s <- %02x (byte %u)\n", addr, reg->name, val, addr );
    if ( reg->data ) {
        reg->data[addr] = val;
    }
    if ( val == 0x81 ) {
        fprintf( stderr, "%c", datReg );
    }
}

void *SerialToStderr( busDevice_t* bus ) {
    GenericBusMapping( bus, "SerDat", 0xFF01, 0xFF01, GenericRegister( "SerDat", &datReg, 1, NULL, NULL ) );
    GenericBusMapping( bus, "SerTxCtl", 0xFF02, 0xFF02, GenericRegister( "SerTxCtl", NULL, 1, SerialRegisterRead, SerialRegisterWrite ) );
}

