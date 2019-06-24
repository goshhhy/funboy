#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "../device/device.h"
#include "mips1.h"

typedef struct mipsInstruction_s {
        uint32_t full;
        struct { //j-type
            uint8_t op : 6;
            uint32_t target : 26;
        };
        struct { //i-type
            uint8_t op_i : 6;
            uint8_t rs : 5;
            uint8_t rt : 5;
            uint16_t imm;
        };
        struct { //r-type
            uint8_t op_r : 6;
            uint8_t rs_r : 5;
            uint8_t rt_r : 5;
            uint8_t rd : 5;
            uint8_t shamt : 5;
            uint8_t funct : 6; 
        };
} mipsInstruction_t;

static uint32_t Read32( busDevice_t *bus, uint32_t addr ) {
    printf( "32 bit read from %x\n", addr );
    return ( bus->Read8( bus, addr + 3, false ) << 24 ) +
            ( bus->Read8( bus, addr + 2, false ) << 16 ) +
            ( bus->Read8( bus, addr + 1, false ) << 8 ) +
            bus->Read8( bus, addr + 0, true );
}


static void Write32( busDevice_t *bus, uint32_t addr, uint32_t val ) {
    printf( "32 bit write to %x\n", addr );
    bus->Write8( bus, addr + 3, val >> 24, false );
    bus->Write8( bus, addr + 2, val >> 16, false );
    bus->Write8( bus, addr + 1, val >> 8, false );
    bus->Write8( bus, addr + 0, val, true );
    printf( "32 bit write complete\n" );
}

static void Reset( mips1_t *cpu ) {
    cpu->pc = 0xbfc00000;
}

static void Step( mips1_t *cpu ) {
    mipsInstruction_t inst;
    inst.full = Read32( cpu->bus, cpu->pc );

    switch ( inst.full & 0xFC00003F ) {
        default:
            printf("unhandled instruction %08x\n", inst.full );
            exit( 100 );
    }
    cpu->pc += 4;
}

mips1_t *Mips1( busDevice_t *bus ) {
	mips1_t *cpu = calloc( 1, sizeof( mips1_t ) );
	cpu->bus = bus;
	cpu->Reset = Reset;
	cpu->Step = Step;
	Reset( cpu );
    printf( "mips1 cpu created\n" );
	return cpu;
}
