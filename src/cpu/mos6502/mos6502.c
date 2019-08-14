

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../../device/device.h"
#include "mos6502.h"

static void Step( mos6502_t *cpu ) {
    
}

static void Reset( mos6502_t *cpu ) {
    
}

mos6502_t *Mos6502( busDevice_t *bus ) {
	mos6502_t *cpu = calloc( 1, sizeof( mos6502_t ) );
	cpu->bus = bus;
	cpu->Reset = Reset;
	cpu->Step = Step;
	Reset( cpu );
    printf( "mos6502 cpu created\n" );
	return cpu;
}