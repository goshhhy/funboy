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
    printf( "32 bit write %x to %x\n", val, addr );
    bus->Write8( bus, addr + 3, val >> 24, false );
    bus->Write8( bus, addr + 2, val >> 16, false );
    bus->Write8( bus, addr + 1, val >> 8, false );
    bus->Write8( bus, addr + 0, val, true );
    printf( "32 bit write complete\n" );
}

static void Write16( busDevice_t *bus, uint32_t addr, uint16_t val ) {
    printf( "16 bit write %x to %x\n", val, addr );
    bus->Write8( bus, addr + 1, val >> 8, false );
    bus->Write8( bus, addr + 0, val, true );
    printf( "16 bit write complete\n" );
}

static void Reset( mips1_t *cpu ) {
    cpu->pc = 0xbfc00000;
}

#define OP( x ) ( x & 0xFC000000 ) >> 26
#define RS( x ) ( x & 0x03E00000 ) >> 21
#define RT( x ) ( x & 0x001F0000 ) >> 16
#define IMM( x ) ( x & 0x0000FFFF )
#define RD( x ) ( x & 0x0000F800 ) >> 11
#define SHAMT( x ) ( x & 0x000007C0 ) >> 6
#define FUNCT( x ) ( x & 0x0000003F )
#define TARGET( x ) ( x & 0x03FFFFFF )

#define SIGNEXTEND16( x ) ((int32_t)( x | ( ( x & 0x8000 ) ? 0xffff0000 : 0x0 ) ) )

static char* Special_DisAsm( uint32_t inst, char *buf ) {
    switch ( FUNCT(inst) ) {
        case 0x00: sprintf( buf, "sll r%u <- r%u << %u", RD(inst), RT(inst), SHAMT(inst) ); break;
        case 0x02: sprintf( buf, "srl r%u <- r%u >> %u", RD(inst), RT(inst), SHAMT(inst) ); break;
        case 0x03: sprintf( buf, "sra r%u <- r%u >> %u", RD(inst), RT(inst), SHAMT(inst) ); break;
        case 0x04: sprintf( buf, "sllv r%u <- r%u << %u", RD(inst), RT(inst), SHAMT(inst) ); break;
        case 0x06: sprintf( buf, "srlv r%u <- r%u >> %u", RD(inst), RT(inst), SHAMT(inst) ); break;
        case 0x07: sprintf( buf, "srav r%u <- r%u >> %u", RD(inst), RT(inst), SHAMT(inst) ); break;
        case 0x08: sprintf( buf, "jr r%u", RS(inst) ); break;
        case 0x09: sprintf( buf, "jalr r%u", RS(inst) ); break;
        case 0x0c: sprintf( buf, "syscall" ); break;
        case 0x0d: sprintf( buf, "break" ); break;
        case 0x10: sprintf( buf, "mfhi" ); break;
        case 0x11: sprintf( buf, "mthi" ); break;
        case 0x12: sprintf( buf, "mflo" ); break;
        case 0x13: sprintf( buf, "mtlo" ); break;
        case 0x18: sprintf( buf, "mult" ); break;
        case 0x19: sprintf( buf, "multu" ); break;
        case 0x1a: sprintf( buf, "div" ); break;
        case 0x1b: sprintf( buf, "divu" ); break;
        case 0x20: sprintf( buf, "add r%u <- r%u + r%u", RD(inst), RS(inst), RT(inst) ); break;
        case 0x21: sprintf( buf, "addu r%u <- r%u + r%u", RD(inst), RS(inst), RT(inst) ); break;
        case 0x22: sprintf( buf, "sub" ); break;
        case 0x23: sprintf( buf, "subu" ); break;
        case 0x24: sprintf( buf, "and r%u <- r%u & r%u", RD(inst), RS(inst), RT(inst) ); break;
        case 0x25: sprintf( buf, "or r%u <- r%u | r%u", RD(inst), RS(inst), RT(inst) ); break;
        case 0x26: sprintf( buf, "xor r%u <- r%u ^ r%u", RD(inst), RS(inst), RT(inst) ); break;
        case 0x27: sprintf( buf, "nor r%u <- r%u !| r%u", RD(inst), RS(inst), RT(inst) ); break;
        case 0x2a: sprintf( buf, "slt r%u = r%u < r%u ? 1 : 0", RD(inst), RS(inst), RT(inst) ); break;
        case 0x2b: sprintf( buf, "sltu r%u = r%u < r%u ? 1 : 0", RD(inst), RS(inst), RT(inst) ); break;
        default: sprintf( buf, "special unk %08x (funct %hhx)\n", inst, FUNCT(inst) ); break;
    }
    return buf;
}

static char* Copz_DisAsm( uint32_t inst, char *buf ) {
    switch ( RS(inst) ) {
        case 0x00: sprintf( buf, "mfcz r%u <- cop%u.r%u", RT(inst), OP(inst) & 0x03, RD(inst) ); break;
        case 0x02: sprintf( buf, "cfcz r%u <- cop%u.cr%u", RT(inst), OP(inst) & 0x03, RD(inst) ); break;
        case 0x04: sprintf( buf, "mtcz r%u -> cop%u.r%u", RT(inst), OP(inst) & 0x03, RD(inst) ); break;
        case 0x06: sprintf( buf, "ctcz r%u -> cop%u.cr%u", RT(inst), OP(inst) & 0x03, RD(inst) ); break;
        case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17: 
        case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f: 
            sprintf( buf, "cop%u %08x\n", OP(inst) & 0x03, inst & 0x03FFFFFF ); break;
        default:
            sprintf( buf, "invalid cop%u\n", OP(inst) & 0x03 );
    }
    return buf;
}

static char* DisAsm( mipsInstruction_t* i ) {
    char* buf = calloc( 1, 256 );
    int inst = i->full;

    switch ( OP(inst) ) {
        case 0x00: Special_DisAsm( inst, buf ); break;
        case 0x01: sprintf( buf, "regimm r%x", RT(inst) ); break;
        case 0x02: sprintf( buf, "jump %x", TARGET(inst) << 2 ); break;
        case 0x03: sprintf( buf, "jal %x", TARGET(inst) << 2 ); break;
        case 0x05: sprintf( buf, "bne r%u r%u %x", RS(inst), RT(inst), ((int32_t)(IMM(inst))) << 2 ); break;
        case 0x06: sprintf( buf, "blez r%u %x", RS(inst), ((int32_t)(IMM(inst))) << 2 ); break;
        case 0x07: sprintf( buf, "bgtz r%u %x", RS(inst), ((int32_t)(IMM(inst))) << 2 ); break;
        case 0x08: sprintf( buf, "addi r%u <- r%u + %i", RT(inst), RS(inst), (int16_t)(IMM(inst)) ); break;
        case 0x09: sprintf( buf, "addiu r%u <- r%u + %i", RT(inst), RS(inst), (int16_t)(IMM(inst)) ); break;
        case 0x0A: sprintf( buf, "slti r%u <- ( r%u < %u )", RT(inst), RS(inst), IMM(inst) ); break;
        case 0x0B: sprintf( buf, "sltiu r%u <- ( r%u < %u )", RT(inst), RS(inst), IMM(inst) ); break;
        case 0x0C: sprintf( buf, "andi r%u <- r%u & 0x%08x", RT(inst), RS(inst), IMM(inst) ); break;
        case 0x0D: sprintf( buf, "ori r%u <- r%u | 0x%08x", RT(inst), RS(inst), IMM(inst) ); break;
        case 0x0E: sprintf( buf, "xori r%u <- r%u ^ 0x%08x", RT(inst), RS(inst), IMM(inst) ); break;
        case 0x0F: sprintf( buf, "lui r%u <- 0x%x", RT(inst), IMM(inst) << 16 ); break;
        case 0x10: 
        case 0x11:
        case 0x12:
        case 0x13: Copz_DisAsm( inst, buf ); break;
        case 0x20: sprintf( buf, "lb r%u <- [r%u + 0x%08x]", RT(inst), RS(inst), IMM(inst) ); break;
        case 0x21: sprintf( buf, "lh r%u <- [r%u + 0x%08x]", RT(inst), RS(inst), IMM(inst) ); break;
        case 0x22: sprintf( buf, "lwl r%u <- [r%u + 0x%08x]", RT(inst), RS(inst), IMM(inst) ); break;
        case 0x23: sprintf( buf, "lw r%u <- [r%u + 0x%08x]", RT(inst), RS(inst), IMM(inst) ); break;
        case 0x24: sprintf( buf, "lbu r%u <- [r%u + 0x%08x]", RT(inst), RS(inst), IMM(inst) ); break;
        case 0x25: sprintf( buf, "lhu r%u <- [r%u + 0x%08x]", RT(inst), RS(inst), IMM(inst) ); break;
        case 0x26: sprintf( buf, "lwr r%u <- [r%u + 0x%08x]", RT(inst), RS(inst), IMM(inst) ); break;
        case 0x28: sprintf( buf, "sb r%u -> [r%u + 0x%08x]", RT(inst), RS(inst), IMM(inst) ); break;
        case 0x29: sprintf( buf, "sh r%u -> [r%u + 0x%08x]", RT(inst), RS(inst), IMM(inst) ); break;
        case 0x2a: sprintf( buf, "swl r%u -> [r%u + 0x%08x]", RT(inst), RS(inst), IMM(inst) ); break;
        case 0x2b: sprintf( buf, "sw r%u -> [r%u + 0x%08x]", RT(inst), RS(inst), IMM(inst) ); break;
        case 0x2e: sprintf( buf, "swr r%u -> [r%u + 0x%08x]", RT(inst), RS(inst), IMM(inst) ); break;
        case 0x30: 
        case 0x31:
        case 0x32:
        case 0x33: sprintf( buf, "lwcz" ); break;
        case 0x38: 
        case 0x39:
        case 0x3a:
        case 0x3b: sprintf( buf, "swcz" ); break;
        default: sprintf( buf, "unk %08x (op %hhx)\n", inst, OP(inst) ); break;
    }
    return buf;
}

static void Special( mips1_t *cpu, uint32_t inst ) {
    switch( FUNCT(inst) ) {
        case 0x00: cpu->gp_regs[RD(inst)] = cpu->gp_regs[RT(inst)] << SHAMT(inst); break;
        case 0x20: cpu->gp_regs[RD(inst)] = (int32_t)cpu->gp_regs[RS(inst)] + (int32_t)cpu->gp_regs[RT(inst)]; break;
        case 0x21: cpu->gp_regs[RD(inst)] = cpu->gp_regs[RS(inst)] + cpu->gp_regs[RT(inst)]; break;
        case 0x24: cpu->gp_regs[RD(inst)] = cpu->gp_regs[RS(inst)] & cpu->gp_regs[RT(inst)]; break;
        case 0x25: cpu->gp_regs[RD(inst)] = cpu->gp_regs[RS(inst)] | cpu->gp_regs[RT(inst)]; break;
        case 0x26: cpu->gp_regs[RD(inst)] = cpu->gp_regs[RS(inst)] ^ cpu->gp_regs[RT(inst)]; break;
        case 0x27: cpu->gp_regs[RD(inst)] = ~(cpu->gp_regs[RS(inst)] | cpu->gp_regs[RT(inst)]); break;
        case 0x2a: cpu->gp_regs[RD(inst)] = ( (int32_t)cpu->gp_regs[RS(inst)] < (int32_t)cpu->gp_regs[RT(inst)] ) ? 1 : 0; break;
        case 0x2b: cpu->gp_regs[RD(inst)] = ( cpu->gp_regs[RS(inst)] < cpu->gp_regs[RT(inst)] ) ? 1 : 0; break;
        default:
            printf("unimplemented special instruction!\n" );
            exit( 100 );
    }
}

static void Copz( mips1_t *cpu, uint32_t inst ) {
    switch( RS(inst) ) {
        case 0x00: cpu->gp_regs[RT(inst)] = cpu->cop_regs[OP(inst) & 0x03][RD(inst)]; break;
        case 0x04: cpu->cop_regs[OP(inst) & 0x03][RD(inst)] = cpu->gp_regs[RT(inst)]; break;
        default:
            printf("unimplemented copz instruction!\n" );
            exit( 100 );
    }
}

static void Step( mips1_t *cpu ) {
    mipsInstruction_t i;
    uint32_t inst, newpc;
    char *dis;

    i.full = Read32( cpu->bus, cpu->pc );
    dis = DisAsm( &i );
    printf( "%u [0x%08x] %08x : %s\n", cpu->steps, cpu->pc, i.full, dis );
    free( dis );

    inst = i.full;
    switch ( OP(inst) ) {
        case 0x00: Special( cpu, inst ); break;
        case 0x02: // jump
            newpc = ( TARGET(inst) << 2 ) + ((cpu->pc + 4) & 0xF0000000 ) - 4;
            cpu->pc += 4;
            printf( "executing branch delay slot\n" );
            Step( cpu );
            printf( "branching\n" );
            cpu->pc = newpc;
            break;
        case 0x03: // jal
            newpc = ( TARGET(inst) << 2 ) + ((cpu->pc + 4) & 0xF0000000 ) - 4;
            cpu->pc += 4;
            printf( "executing branch delay slot\n" );
            Step( cpu );
            printf( "branching\n" );
            cpu->gp_regs[31] = (cpu->pc + 4);
            cpu->pc = newpc;
            break;
        case 0x05: // bne
            if ( cpu->gp_regs[RS(inst)] != cpu->gp_regs[RT(inst)] ) {
                newpc = ( SIGNEXTEND16( (int32_t)IMM(inst) ) << 2 ) + (((int32_t)cpu->pc + 4) ) - 4;
                cpu->pc += 4;
                printf( "executing branch delay slot\n" );
                Step( cpu );
                printf( "branching\n" );
                cpu->pc = newpc;
            }
            break;
        case 0x08: cpu->gp_regs[RT(inst)] = cpu->gp_regs[RS(inst)] + IMM(inst); break; //todo: signed?
        case 0x09: cpu->gp_regs[RT(inst)] = cpu->gp_regs[RS(inst)] + IMM(inst); break; 
        case 0x0A: cpu->gp_regs[RT(inst)] = ( cpu->gp_regs[RS(inst)] < IMM(inst) ? 1 : 0); break; //todo: signed?
        case 0x0B: cpu->gp_regs[RT(inst)] = ( cpu->gp_regs[RS(inst)] < IMM(inst) ? 1 : 0); break;
        case 0x0C: cpu->gp_regs[RT(inst)] = cpu->gp_regs[RS(inst)] & IMM(inst); break; 
        case 0x0D: cpu->gp_regs[RT(inst)] = cpu->gp_regs[RS(inst)] | IMM(inst); break;
        case 0x0E: cpu->gp_regs[RT(inst)] = cpu->gp_regs[RS(inst)] ^ IMM(inst); break; 
        case 0x0F: cpu->gp_regs[RT(inst)] = IMM(inst) << 16; break;
        case 0x10: case 0x11: case 0x12: case 0x13: Copz( cpu, inst );
        case 0x23: cpu->gp_regs[RT(inst)] = Read32( cpu->bus, SIGNEXTEND16(IMM(inst)) + (int32_t)cpu->gp_regs[RS(inst)]);
        case 0x28: cpu->bus->Write8( cpu->bus, cpu->gp_regs[RS(inst)] + IMM(inst), cpu->gp_regs[RT(inst)] & 0xFF , true ); break;
        case 0x29: Write16( cpu->bus, cpu->gp_regs[RS(inst)] + IMM(inst), cpu->gp_regs[RT(inst)] & 0xFF ); break;
        case 0x2B: Write32( cpu->bus, cpu->gp_regs[RS(inst)] + IMM(inst), cpu->gp_regs[RT(inst)] ); break;
        default:
            printf("unimplemented instruction!\n" );
            exit( 100 );
    }
    cpu->gp_regs[0] = 0;
    cpu->pc += 4;
    cpu->steps++;
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
