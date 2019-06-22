#include <stdio.h>
#include <stdint.h>

#include "../device/device.h"

// contains all data needed for mips processor emulation, and callback functions
typedef struct mips1_s {
    uint32_t gp_regs[32];
    union {
       uint32_t sr;
       struct {
           uint8_t cu3: 1;
           uint8_t cu2: 1;
           uint8_t cu1: 1;
           uint8_t cu0: 1;
           uint8_t zero0: 2;
           uint8_t re: 1;
           uint8_t zero1: 2;
           uint8_t bev: 1;
           uint8_t ts: 1;
           uint8_t pe: 1;
           uint8_t cm: 1;
           uint8_t pz: 1;
           uint8_t swc: 1;
           uint8_t isc: 1;
           uint8_t im: 8;
           uint8_t zero2: 2;
           uint8_t kuo: 1;
           uint8_t ieo: 1;
           uint8_t kup: 1;
           uint8_t iep: 1;
           uint8_t kuc: 1;
           uint8_t iec: 1;
       };
    };
    uint32_t pc;
    union {
        uint64_t mf;
        union {
            uint32_t mfhi;
            uint32_t mflo;
        };
    };
    busDevice_t* bus;
} mips1_t;

typedef union mipsInstruction_s {
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

uint32_t Read32( busDevice_t *bus, uint32_t addr ) {
    return ( bus->Read8( bus, addr + 3 ) << 24 ) +
            ( bus->Read8( bus, addr + 2 ) << 16 ) +
            ( bus->Read8( bus, addr + 1 ) << 8 ) +
            bus->Read8( bus, addr + 0 );
}


void Write( busDevice_t *bus, uint32_t addr, uint32_t val ) {
    bus->Write8( bus, addr + 3, val >> 24 );
    bus->Write8( bus, addr + 2, val >> 16 );
    bus->Write8( bus, addr + 1, val >> 8 );
    bus->Write8( bus, addr + 0, val );
}

void Reset( mips1_t *cpu ) {
    cpu->pc = 0xbfc00000;
}

void Step( mips1_t *cpu ) {
    mipsInstruction_t inst;
    inst.full = Read32( cpu->bus, cpu->pc );

    switch ( inst.full & 0xFC00003F ) {
        default:
            printf("unhandled instruction %08x\n", inst.full );
            break;
    }
    cpu->pc += 4;
}