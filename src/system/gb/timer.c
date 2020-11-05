#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "version.h"
#include "device.h"
#include "sm83.h"
#include "timer.h"


typedef struct regInfo_s {
    char* name;
    size_t len;
    unsigned long* data;
} regInfo_t;

static int enabled;
static unsigned char divReg;
static unsigned char divSubcount;
static unsigned char countReg;
static unsigned char countSubcount;
static unsigned char divisor;
static unsigned char modulo;
static unsigned char control;
static int overflowed;

static void DivRegisterWrite( busDevice_t *dev, unsigned long addr, unsigned char val, int final ) {
    regInfo_t *reg;

    if ( !dev || !dev->data )
        return;
    reg = dev->data;

    if ( addr > reg->len ) {
        fprintf( stderr, "warning: DivRegisterWrite: address out of bounds\n" );
        return;
    }
    printf( "write register [0x%08lx]%s <- %02x (byte %lu), becomes 0\n", addr, reg->name, val, addr );
    divReg = 0;
    divSubcount = 0;
}

static void ControlRegisterWrite( busDevice_t *dev, unsigned long addr, unsigned char val, int final ) {
    regInfo_t *reg;

    if ( !dev || !dev->data )
        return;
    reg = dev->data;

    if ( addr > reg->len ) {
        fprintf( stderr, "warning: DivRegisterWrite: address out of bounds\n" );
        return;
    }

    printf( "write register [0x%08lx]%s <- %02hx (byte %lu)\n", addr, reg->name, val, addr );
    control = val & 0x07;

    enabled = ( ( control & 4 ) != 0 );
    
    fprintf( stderr, "timer set! ctrl %02x\n", control & 0x03 );
}

static void Step( gbTimer_t *self ) {
    unsigned char selectors_pre[4] = { 0, 0, 0, 0 }, selectors_post[4] = { 0, 0, 0, 0 };
    
    if ( enabled && overflowed ) {
            /* printf( "timer interrupt!\n" ); */
            /* fprintf( stderr, "timer interrupt!\n" );  */
            self->cpu->Interrupt( self->cpu, 2 );
        countReg = modulo;
        overflowed = 0;
    } 

    selectors_pre[0] = ( divReg & 0x04 ) >> 2;
    selectors_pre[1] = ( divSubcount & 0x10 ) >> 4;
    selectors_pre[2] = ( divSubcount & 0x40 ) >> 6;
    selectors_pre[3] = ( divReg & 0x01 );

    divSubcount++;
    if ( divSubcount == 0 ) {
        divReg++;
    }

    if ( !enabled )
        return;

    selectors_post[0] = ( divReg & 0x04 ) >> 2;
    selectors_post[1] = ( divSubcount & 0x10 ) >> 4;
    selectors_post[2] = ( divSubcount & 0x40 ) >> 6;
    selectors_post[3] = ( divReg & 0x01 );

    if ( selectors_pre[control & 0x03] != selectors_post[control & 0x03] ) {
        countReg++;
        if ( countReg == 0 ) {
            overflowed = 1;
        }
    }
}

gbTimer_t *GbTimer( busDevice_t *bus, sm83_t *cpu ) {
    gbTimer_t *timer = malloc( sizeof( gbTimer_t ) );
    timer->Step = Step;
    timer->cpu = cpu;

    enabled = 0;
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

