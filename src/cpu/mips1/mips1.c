#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "../../device/device.h"
#include "mips1.h"


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

#define OP( x ) x >> 26
#define RS( x ) ( x & 0x03E00000 ) >> 21
#define RT( x ) ( x & 0x001F0000 ) >> 16
#define IMM( x ) ( x & 0x0000FFFF )
#define RD( x ) ( x & 0x0000F800 ) >> 10
#define SHAMT( x ) ( x & 0x000007C0 ) >> 6
#define FUNCT( x ) ( x & 0x0000003F )
#define TARGET( x ) ( x & 0x03FFFFFF )

static char* DisAsm( mipsInstruction_t* i ) {
    char* buf = calloc( 1, 256 );
    int inst = i->full;

    switch ( OP(inst) ) {
        case 0x00: sprintf( buf, "special f.%x", FUNCT(inst) ); break;
        case 0x01: sprintf( buf, "regimm r%x", RT(inst) ); break;
        case 0x02: sprintf( buf, "jump %x", TARGET(inst) << 2 ); break;
        case 0x03: sprintf( buf, "jal %x", TARGET(inst) << 2 ); break;
        case 0x05: sprintf( buf, "bne r%u r%u %x", RS(inst), RT(inst), ((int32_t)(IMM(inst))) << 2 ); break;
        case 0x06: sprintf( buf, "blez r%u %x", RS(inst), ((int32_t)(IMM(inst))) << 2 ); break;
        case 0x07: sprintf( buf, "bgtz r%u %x", RS(inst), ((int32_t)(IMM(inst))) << 2 ); break;
        case 0x08: sprintf( buf, "addi r%u <- r%u + %i\n", RT(inst), RS(inst), (int16_t)(IMM(inst)) );
        case 0x09: sprintf( buf, "addiu r%u <- r%u + %i\n", RT(inst), RS(inst), (int16_t)(IMM(inst)) );
        case 0x0A: sprintf( buf, "slti r%u <- ( r%u < %u )", RT(inst), RS(inst), IMM(inst) ); break;
        case 0x0B: sprintf( buf, "sltiu r%u <- ( r%u < %u )", RT(inst), RS(inst), IMM(inst) ); break;
        case 0x0C: sprintf( buf, "andi r%u <- r%u & 0x%08x", RT(inst), RS(inst), IMM(inst) );
        case 0x0D: sprintf( buf, "ori r%u <- r%u | 0x%08x", RT(inst), RS(inst), IMM(inst) );
        case 0x0E: sprintf( buf, "xori r%u <- r%u ^ 0x%08x", RT(inst), RS(inst), IMM(inst) );
        case 0x0F: sprintf( buf, "lui r%u <- 0x%x", RT(inst), IMM(inst) << 16 ); break;
        case 0x10: 
        case 0x11:
        case 0x12:
        case 0x13: sprintf( buf, "copz" ); break;
        case 0x20: sprintf( buf, "lb r%u <- [r%u + 0x%08x]", RT(inst), RS(inst), IMM(inst) );
        case 0x21: sprintf( buf, "lh r%u <- [r%u + 0x%08x]", RT(inst), RS(inst), IMM(inst) );
        case 0x22: sprintf( buf, "lwl r%u <- [r%u + 0x%08x]", RT(inst), RS(inst), IMM(inst) );
        case 0x23: sprintf( buf, "lw r%u <- [r%u + 0x%08x]", RT(inst), RS(inst), IMM(inst) );
        case 0x24: sprintf( buf, "lbu r%u <- [r%u + 0x%08x]", RT(inst), RS(inst), IMM(inst) );
        case 0x25: sprintf( buf, "lhu r%u <- [r%u + 0x%08x]", RT(inst), RS(inst), IMM(inst) );
        case 0x26: sprintf( buf, "lwr r%u <- [r%u + 0x%08x]", RT(inst), RS(inst), IMM(inst) );
        case 0x28: sprintf( buf, "sb r%u -> [r%u + 0x%08x]", RT(inst), RS(inst), IMM(inst) );
        case 0x29: sprintf( buf, "sh r%u -> [r%u + 0x%08x]", RT(inst), RS(inst), IMM(inst) );
        case 0x2a: sprintf( buf, "swl r%u -> [r%u + 0x%08x]", RT(inst), RS(inst), IMM(inst) );
        case 0x2b: sprintf( buf, "sw r%u -> [r%u + 0x%08x]", RT(inst), RS(inst), IMM(inst) );
        case 0x2e: sprintf( buf, "swr r%u -> [r%u + 0x%08x]", RT(inst), RS(inst), IMM(inst) );
        case 0x30: 
        case 0x31:
        case 0x32:
        case 0x33: sprintf( buf, "lwcz" ); break;
        case 0x38: 
        case 0x39:
        case 0x3a:
        case 0x3b: sprintf( buf, "swcz" ); break;
        default:
            sprintf( buf, "unk %08x\n", inst );
            break;
    }
    return buf;
}

static void Step( mips1_t *cpu ) {
    mipsInstruction_t inst;
    char *dis;

    inst.full = Read32( cpu->bus, cpu->pc );
    dis = DisAsm( &inst );
    printf( "[0x%08x] %08x : %s\n", cpu->pc, inst.full, dis );
    free( dis );

    switch ( inst.full & 0xFC00003F ) {
        default:
            printf("unimplemented instruction!\n" );
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
