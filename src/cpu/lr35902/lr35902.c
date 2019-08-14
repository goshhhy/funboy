

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../../device/device.h"
#include "lr35902.h"

static void Step( lr35902_t *cpu ) {
    
}

static void Reset( lr35902_t *cpu ) {
    
}

lr35902_t *Lr35902( busDevice_t *bus ) {
	lr35902_t *cpu = calloc( 1, sizeof( lr35902_t ) );
	cpu->bus = bus;
	cpu->Reset = Reset;
	cpu->Step = Step;
	Reset( cpu );
    printf( "lr35902 cpu created\n" );
	return cpu;
}