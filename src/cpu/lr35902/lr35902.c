

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../../device/device.h"
#include "lr35902.h"

#define write8( cpu, addr, val, final ) cpu->bus->Write8( cpu->bus, addr, val, final )
#define read8( cpu, addr, final ) cpu->bus->Read8( cpu->bus, addr, final )
#define read16( cpu, addr, final ) ( ( cpu->bus->Read8( cpu->bus, addr + 1, false ) << 8 ) + cpu->bus->Read8( cpu->bus, addr, final ) )

typedef enum {
    LD_MODE_B  = 0,
    LD_MODE_C  = 1,
    LD_MODE_D  = 2,
    LD_MODE_E  = 3,
    LD_MODE_H  = 4,
    LD_MODE_L  = 5,
    LD_MODE_HL = 6,
    LD_MODE_A  = 7,
} ldMode_t;;

static void nop( lr35902_t *cpu ) {
    cpu->pc++;
}

void SetFlags( lr35902_t *cpu, uint8_t z, uint8_t n, uint8_t h, uint8_t c ) {
    cpu->fz = z;
    cpu->fn = n;
    cpu->fh = h;
    cpu->fc = c;
}

bool NzNc( lr35902_t *cpu ) {
    bool cond = ( ( ( cpu->op.p & 1 ) == 0 ) ? cpu->fz : cpu->fc );
    return ( cpu->op.q == 0 ) ? !cond : cond;
}

// REGISTER READ/WRITES

static uint8_t read_r( lr35902_t *cpu, uint8_t r ) {
    uint8_t *ri[8] = {&cpu->b, &cpu->c,  &cpu->d,  &cpu->e,  &cpu->h,  &cpu->l, NULL,  &cpu->a };
    return r == LD_MODE_HL ? read8( cpu, cpu->hl, true ) : *ri[r];
}

static void write_r( lr35902_t *cpu, uint8_t val, uint8_t r ) {
    uint8_t *ri[8] = {&cpu->b, &cpu->c,  &cpu->d,  &cpu->e,  &cpu->h,  &cpu->l, NULL,  &cpu->a };
    if ( r == LD_MODE_HL )
        write8( cpu, cpu->hl, val, true );
    else
        *ri[r] = val;
}

static uint16_t read_rp( lr35902_t *cpu, uint8_t r ) {
    uint16_t *ri[4] = {&cpu->bc, &cpu->de,  &cpu->hl,  &cpu->sp };
    return *ri[r];
}

static void write_rp( lr35902_t *cpu, uint16_t val, uint8_t r ) {
    uint16_t *ri[4] = {&cpu->bc, &cpu->de,  &cpu->hl,  &cpu->sp };
    *ri[r] = val;
}

static uint16_t read_rp2( lr35902_t *cpu, uint8_t r ) {
    uint16_t *ri[4] = {&cpu->bc, &cpu->de,  &cpu->hl,  &cpu->af };
    return *ri[r];
}

static void write_rp2( lr35902_t *cpu, uint16_t val, uint8_t r ) {
    uint16_t *ri[4] = {&cpu->bc, &cpu->de,  &cpu->hl,  &cpu->af };
    *ri[r] = val;
}

// OPCODES

static void ld( lr35902_t *cpu ) {
    write_r( cpu, read_r( cpu, cpu->op.z ), cpu->op.y );
}

static void ldd8( lr35902_t *cpu ) {
    write_r( cpu, read8( cpu, cpu->pc++ + 1, false ), cpu->op.y );
}

static void add( lr35902_t *cpu ) {
    printf( "add unimplemented\n" );
    exit(1);
}

static void adc( lr35902_t *cpu ) {
    printf( "adc unimplemented\n" );
    exit(1);
}

static void sub( lr35902_t *cpu ) {
    printf( "sub unimplemented\n" );
    exit(1);
}

static void sbc( lr35902_t *cpu ) {
    printf( "sbc unimplemented\n" );
    exit(1);
}

static void and( lr35902_t *cpu ) {
    printf( "and unimplemented\n" );
    exit(1);
}

static void xor( lr35902_t *cpu ) {
    cpu->fl = 0;
    cpu->fz = ( ( cpu->a = ( cpu->a ^ read_r( cpu, cpu->op.z ) ) ) == 0 );
}

static void or( lr35902_t *cpu ) {
    printf( "or unimplemented\n" );
    exit(1);
}

static void cp( lr35902_t *cpu ) {
    printf( "cp unimplemented\n" );
    exit(1);
}

static void inc( lr35902_t *cpu ) {
    uint8_t in = read_r( cpu, cpu->op.y ), res = in + 1;
    SetFlags( cpu, res == 0, 1, ( res & 0xf ) < ( in & 0xf), cpu->fc );
    write_r( cpu, res, cpu->op.y );
}

static void dec( lr35902_t *cpu ) {
    uint8_t in = read_r( cpu, cpu->op.y ), res = in - 1;
    SetFlags( cpu, res == 0, 1, ( res & 0x0f ) > ( in & 0x0f), cpu->fc );
    write_r( cpu, res, cpu->op.y );
}

static void in16( lr35902_t *cpu ) {
    printf( "cp unimplemented\n" );
    exit(1);
}

static void de16( lr35902_t *cpu ) {
    printf( "de16 unimplemented\n" );
    exit(1);
}

static void ld16( lr35902_t *cpu ) {
    write_rp( cpu, read16( cpu, ( cpu->pc+1 ), true ), cpu->op.p );
    cpu->pc+=2;
}

static void li16( lr35902_t *cpu ) {
    write8( cpu, cpu->op.p < 2 ? read_rp( cpu, cpu->op.p ) : ( cpu->op.p == 2 ? cpu->hl++ : cpu->hl-- ), cpu->a, true );
}

static void add16( lr35902_t *cpu ) {
    printf( "add16 unimplemented\n" );
    exit(1);
}

static void la16( lr35902_t *cpu ) {
    printf( "la16 unimplemented\n" );
    exit(1);
}

static void jp( lr35902_t* cpu ) {
    uint16_t dest = read16( cpu, cpu->pc + 1, true );
    printf( "jump to %04x\n", dest );
    cpu->pc = dest - 1;
}

static void jrx( lr35902_t* cpu ) {
    uint16_t dest = cpu->pc + (int8_t)read8( cpu, cpu->pc + 1, true );
    if ( NzNc( cpu ) )
        cpu->pc = dest;
    cpu->pc++;
}

static void hlt( lr35902_t *cpu ) {
    printf( "hlt unimplemented\n" );
    exit(1);
}

static void rst( lr35902_t *cpu ) {
    printf( "rst unimplemented\n" );
    exit(1);
}

static void pop( lr35902_t *cpu ) {
    printf( "pop unimplemented\n" );
    exit(1);
}

static void psh( lr35902_t *cpu ) {
    printf( "psh unimplemented\n" );
    exit(1);
}

static void edi( lr35902_t *cpu ) {
    printf( "edi unimplemented\n" );
    exit(1);
}

static void stop( lr35902_t *cpu ) {
    printf( "stop unimplemented\n" );
    exit(1);
}

static void bad( lr35902_t *cpu ) {
    printf( "bad instruction %02x\n", cpu->op.full );
    exit( 1 );
}

void (*ops[256])( lr35902_t* cpu ) = {
    nop,  ld16, li16, in16,  inc,  dec,  ldd8, bad,     bad,  add16,la16, de16,   inc,  dec,  ldd8, bad, 
    stop, ld16, li16, in16,  inc,  dec,  ldd8, bad,     bad,  add16,la16, de16,   inc,  dec,  ldd8, bad, 
    jrx,  ld16, li16, in16,  inc,  dec,  ldd8, bad,     bad,  add16,la16, de16,   inc,  dec,  ldd8, bad, 
    jrx,  ld16, li16, in16,  inc,  dec,  ldd8, bad,     bad,  add16,la16, de16,   inc,  dec,  ldd8, bad, 

    ld,   ld,   ld,   ld,     ld,   ld,   ld,   ld,     ld,   ld,   ld,   ld,     ld,   ld,   ld,   ld,  
    ld,   ld,   ld,   ld,     ld,   ld,   ld,   ld,     ld,   ld,   ld,   ld,     ld,   ld,   ld,   ld,  
    ld,   ld,   ld,   ld,     ld,   ld,   ld,   ld,     ld,   ld,   ld,   ld,     ld,   ld,   ld,   ld,  
    ld,   ld,   ld,   ld,     ld,   ld,   hlt,  ld,     ld,   ld,   ld,   ld,     ld,   ld,   ld,   ld,  

    add,  add,  add,  add,    add,  add,  add,  add,    adc,  adc,  adc,  adc,    adc,  adc,  adc,  adc, 
    sub,  sub,  sub,  sub,    sub,  sub,  sub,  sub,    sbc,  sbc,  sbc,  sbc,    sbc,  sbc,  sbc,  sbc, 
    and,  and,  and,  and,    and,  and,  and,  and,    xor,  xor,  xor,  xor,    xor,  xor,  xor,  xor, 
    or,   or,   or,   or,     or,   or,   or,   or,     cp,   cp,   cp,   cp,     cp,   cp,   cp,   cp,  

    bad,  pop,  bad,  jp,     bad,  psh,  bad,  rst,    bad,  bad,  bad,  bad,    bad,  bad,  bad,  rst, 
    bad,  pop,  bad,  bad,    bad,  psh,  bad,  rst,    bad,  bad,  bad,  bad,    bad,  bad,  bad,  rst, 
    bad,  pop,  bad,  bad,    bad,  psh,  bad,  rst,    bad,  bad,  bad,  bad,    bad,  bad,  bad,  rst, 
    bad,  pop,  bad,  edi,    bad,  psh,  bad,  rst,    bad,  bad,  bad,  edi,    bad,  bad,  bad,  rst, 
};

static void Step( lr35902_t *cpu ) {
    cpu->op.full = cpu->bus->Read8( cpu->bus, cpu->pc, false );
    uint16_t imm16 = read16(cpu, cpu->pc+1, false);
    printf("[%04x] af:%04x bc:%04x de: %04x hl: %04x op: %02x imm16: %02x\n", cpu->pc, cpu->af, cpu->bc, cpu->de, cpu->hl, cpu->op.full, imm16 );

    ops[cpu->op.full]( cpu );
    cpu->pc++;
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