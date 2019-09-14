#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "../../version.h"
#include "../../device/device.h"
#include "../../cpu/sm83/sm83.h"
#include "timer.h"


typedef struct regInfo_s {
    char* name;
    size_t len;
    uint32_t* data;
} regInfo_t;

static bool enabled;
static uint8_t divReg;
static uint8_t divSubcount;
static uint8_t countReg;
static uint8_t countSubcount;
static uint8_t divisor;
static uint8_t modulo;
static uint8_t control;

static void DivRegisterWrite( busDevice_t *dev, uint32_t addr, uint8_t val, bool final ) {
    regInfo_t *reg;

    if ( !dev || !dev->data )
        return;
    reg = dev->data;

    if ( addr > reg->len ) {
        fprintf( stderr, "warning: DivRegisterWrite: address out of bounds\n" );
        return;
    }

    fprintf( stderr, "write register [0x%08x]%s <- %02x (byte %u), becomes 0\n", addr, reg->name, val, addr );
    divReg = 0;
}

static void ControlRegisterWrite( busDevice_t *dev, uint32_t addr, uint8_t val, bool final ) {
    regInfo_t *reg;

    if ( !dev || !dev->data )
        return;
    reg = dev->data;

    if ( addr > reg->len ) {
        fprintf( stderr, "warning: DivRegisterWrite: address out of bounds\n" );
        return;
    }

    fprintf( stderr, "write register [0x%08x]%s <- %02x (byte %u)\n", addr, reg->name, val, addr );
    control = val & 0x07;

    enabled = ( ( control & 4 ) != 0 );
    
    switch ( control & 0x03 ) {
        case 0: divisor = 64; break;
        case 1: divisor = 1; break;
        case 2: divisor = 4; break;
        case 3: divisor = 16; break;
        default: fprintf( stderr, "universe is broken in timer\n"); exit( -1 );
    }
}

static void Step( gbTimer_t *self ) {
    if ( !enabled )
        return;
    if ( divSubcount++ > 15 ) {
        divSubcount = 0;
        divReg++;
    }
    if ( countSubcount++ >= divisor ) {
        countSubcount = 0;
        if ( ( countReg++ ) == 0 ) {
            self->cpu->Interrupt( self->cpu, 2 );
        }
    }
}

gbTimer_t *GbTimer( busDevice_t *bus, sm83_t *cpu ) {
    gbTimer_t *timer = malloc( sizeof( gbTimer_t ) );
    timer->Step = Step;
    timer->cpu = cpu;

    enabled = false;
    divReg = 0;
    divSubcount = 0;
    countReg = 0;
    countSubcount = 0;
    divisor = 64;
    modulo = 0;

    GenericBusMapping( bus, "TmrDiv", 0xFF04, 0xFF04, GenericRegister( "TmrDiv", &divReg, 1, NULL, DivRegisterWrite ) );
    GenericBusMapping( bus, "TmrCount", 0xFF05, 0xFF05, GenericRegister( "TmrCount", &countReg, 1, NULL, NULL ) );
    GenericBusMapping( bus, "TmrMod", 0xFF06, 0xFF06, GenericRegister( "TmrMod", &modulo, 1, NULL, NULL ) );
    GenericBusMapping( bus, "TmrCtrl", 0xFF07, 0xFF07, GenericRegister( "TmrCtrl", &control, 1, NULL, ControlRegisterWrite ) );
    return timer;
}

