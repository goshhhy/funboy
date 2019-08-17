#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../../device/device.h"
#include "mos6502.h"

#define read8( cpu, addr, final ) cpu->bus->Read8( cpu, addr, final )
#define read16( cpu, addr, final ) ( ( cpu->bus->Read8( cpu, addr + 1, false ) << 8 ) + cpu->bus->Read8( cpu, addr, final ) )


// ADDRESS MODES
static void zp8xi( mos6502_t *cpu ) {
}
static void zp8( mos6502_t *cpu ) {
}
static void imm8( mos6502_t *cpu ) {
}
static void abs( mos6502_t *cpu ) {
}
static void zp8y( mos6502_t *cpu ) {
}
static void zp8x( mos6502_t *cpu ) {
}
static void absx( mos6502_t *cpu ) {
}
static void absy( mos6502_t *cpu ) {
}
static void acc( mos6502_t *cpu ) {
}

// OPS
static void nop( mos6502_t *cpu ) {

}

// logical operations
static void log( mos6502_t *cpu ) {

}

static void bra( mos6502_t *cpu ) {
	
}

// LOOKUP TABLES

void *g_modetbl[4][8] = {
	{ imm8,  zp8,  imm8, abs,  imm8, zp8x, imm8, absx },
	{ zp8xi, zp8,  imm8, abs,  zp8y, zp8x, absy, absx },
	{ imm8,  zp8,  acc,  abs,  imm8, zp8x, imm8, absx },
	{ zp8xi, zp8,  acc,  abs,  zp8y, zp8x, absy, absx },
};

void *g_optbl[4][8] = {
	{ nop, nop, nop, nop, nop, nop, nop, nop },
	{ nop, nop, nop, nop, nop, nop, nop, nop },
	{ nop, nop, nop, nop, nop, nop, nop, nop },
	{ nop, nop, nop, nop, nop, nop, nop, nop },
};

// EXTERNAL INTERFACES

static void Step( mos6502_t *cpu ) {
	mos6502_instruction_t op;
	op.full = cpu->op.full = cpu->bus->Read8( cpu, cpu->pc, false );
	uint8_t branch = op.b == 0x10;
	uint8_t pptid = op.b == 0x08;
	uint8_t cst = op.b == 0x18;
	cpu->target_reg = (op.b7&(~op.b0))+(op.b7&((op.full<0xE0)&((op.g)==0)));
	cpu->mode = g_modetbl[op.g][op.am];
	

	g_optbl[op.g][op.sb]( cpu );
}

static void Reset( mos6502_t *cpu ) {

}

mos6502_t *Mos6502( busDevice_t *bus ) {
	mos6502_t *cpu = calloc( 1, sizeof( mos6502_t ) );
	cpu->bus = bus;
	cpu->Reset = Reset;
	cpu->Step = Step;
	Reset( cpu );
    printf( "[mos6502] cpu created\n" );
	return cpu;
} 