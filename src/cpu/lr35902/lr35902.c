/* please dont use this code as a "good example" of anything 
 * most people will probably hate it, i know */

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
    uint16_t *ri[4] = {&cpu->bc, &cpu->de,  &cpu->hl, &cpu->af };
    *ri[r] = val & ( r == 3 ? 0xFFF0 : 0xFFFF );
}

// OPCODES

static void ld( lr35902_t *cpu ) {
    write_r( cpu, read_r( cpu, cpu->op.z ), cpu->op.y );
}

static void ldd8( lr35902_t *cpu ) {
    write_r( cpu, read8( cpu, cpu->pc++ + 1, false ), cpu->op.y );
}

static void add( lr35902_t *cpu ) {
    uint8_t other = ( cpu->op.x == 3 ) ? read8( cpu, ++cpu->pc, true ) : read_r( cpu, cpu->op.z );
    uint8_t orig = cpu->a, res = cpu->a = cpu->a + other;
    SetFlags( cpu, res == 0, 0, ( res & 0xf ) < ( orig & 0xf ), res < orig );
}

static void adc( lr35902_t *cpu ) {
    uint8_t other = ( cpu->op.x == 3 ) ? read8( cpu, ++cpu->pc, true ) : read_r( cpu, cpu->op.z );
    uint8_t orig = cpu->a, res = cpu->a = cpu->a + other + cpu->fc;
    SetFlags( cpu, res == 0, 0, ( other & 0xf ) + ( orig & 0xf ) + cpu->fc > 0xf, ((uint16_t)orig + (uint16_t)other + (uint16_t)cpu->fc) > 0xFF);
}

static void sub( lr35902_t *cpu ) {
    uint8_t other = ( cpu->op.x == 3 ) ? read8( cpu, ++cpu->pc, true ) : read_r( cpu, cpu->op.z );
    uint8_t orig = cpu->a, res = cpu->a = cpu->a - other;
    SetFlags( cpu, res == 0, 1, ( res & 0xf ) > ( orig & 0xf ), res > orig );
}

static void sbc( lr35902_t *cpu ) {
    uint8_t other = ( cpu->op.x == 3 ) ? read8( cpu, ++cpu->pc, true ) : read_r( cpu, cpu->op.z );
    uint8_t orig = cpu->a, res = cpu->a = ( cpu->a - other ) - cpu->fc;
    SetFlags( cpu, res == 0, 1, (( other & 0xf ) + cpu->fc ) > ( orig & 0xf ), ((orig - other) - cpu->fc) < 0 );
}

static void and( lr35902_t *cpu ) {
    uint8_t other = ( cpu->op.x == 3 ) ? read8( cpu, ++cpu->pc, true ) : read_r( cpu, cpu->op.z );
    SetFlags( cpu, ( ( cpu->a = ( cpu->a & other ) ) == 0 ), 0, 1, 0 );
}

static void xor( lr35902_t *cpu ) {
    uint8_t other = ( cpu->op.x == 3 ) ? read8( cpu, ++cpu->pc, true ) : read_r( cpu, cpu->op.z );
    SetFlags( cpu, ( ( cpu->a = ( cpu->a ^ other ) ) == 0 ), 0, 0, 0 );
}

static void or( lr35902_t *cpu ) {
    uint8_t other = ( cpu->op.x == 3 ) ? read8( cpu, ++cpu->pc, true ) : read_r( cpu, cpu->op.z );
    SetFlags( cpu, ( ( cpu->a = ( cpu->a | other ) ) == 0 ), 0, 0, 0 );
}

static void cp( lr35902_t *cpu ) {
    uint8_t other = ( cpu->op.x == 3 ) ? read8( cpu, ++cpu->pc, true ) : read_r( cpu, cpu->op.z );
    uint8_t res = cpu->a - other;
    SetFlags( cpu, ( res == 0 ), 1, ( res & 0x0f ) > ( cpu->a & 0x0f ), ( res ) > ( cpu->a ) );
}

static void cpl( lr35902_t *cpu ) {
    cpu->a = ~cpu->a;
    SetFlags( cpu, cpu->fz, 1, 1, cpu->fc );
}

static void inc( lr35902_t *cpu ) {
    uint8_t in = read_r( cpu, cpu->op.y ), res = in + 1;
    SetFlags( cpu, res == 0, 0, ( res & 0xf ) < ( in & 0xf), cpu->fc );
    write_r( cpu, res, cpu->op.y );
}

static void dec( lr35902_t *cpu ) {
    uint8_t in = read_r( cpu, cpu->op.y ), res = in - 1;
    SetFlags( cpu, res == 0, 1, ( res & 0x0f ) > ( in & 0x0f), cpu->fc );
    write_r( cpu, res, cpu->op.y );
}

static void ccf( lr35902_t *cpu ) {
    SetFlags( cpu, cpu->fz, 0, 0, cpu->fc ^ 1 );
}

static void scf( lr35902_t *cpu ) {
    SetFlags( cpu, cpu->fz, 0, 0, 1 );
}

static void daa( lr35902_t *cpu ) {
    bool carry = cpu->fc;
    if ( cpu->fn ) {
        cpu->a = ( cpu->a - ( cpu->fc ? 0x60 : 0 ) ) - ( cpu->fh ? 0x6 : 0 );
    } else {
        if ( cpu->fc || ( cpu->a > 0x99 ) ) {
            cpu->a += 0x60;
            carry = 1;
        }
        if ( cpu->fh || ( ( cpu->a & 0xf ) > 0x9 ) ) {
            cpu->a += 0x6;
        }
    }
    SetFlags( cpu, cpu->a == 0, cpu->fn, 0, carry );
}

static void rlca( lr35902_t *cpu ) {
    SetFlags( cpu, 0, 0, 0, ( cpu->a & 0x80 ) >> 7 );
    cpu->a = ( cpu->a << 1 ) + cpu->fc;
}

static void rla( lr35902_t *cpu ) {
    uint8_t newc = ( cpu->a & 0x80 ) >> 7;
    cpu->a = ( cpu->a << 1 ) + cpu->fc;
    SetFlags( cpu, 0, 0, 0, newc );
}

static void rrca( lr35902_t *cpu ) {
    SetFlags( cpu, 0, 0, 0, ( cpu->a & 0x01 ) );
    cpu->a = ( cpu->a >> 1 ) | ( ( cpu->a & 0x01 ) << 7 );
}

static void rra( lr35902_t *cpu ) {
    uint8_t newc = ( cpu->a & 0x01 );
    cpu->a = ( cpu->a >> 1 ) + ( cpu->fc << 7 );
    SetFlags( cpu, 0, 0, 0, newc );
}

static void in16( lr35902_t *cpu ) {
    write_rp( cpu, read_rp( cpu, cpu->op.p ) + 1, cpu->op.p );
}

static void de16( lr35902_t *cpu ) {
    write_rp( cpu, read_rp( cpu, cpu->op.p ) - 1, cpu->op.p );
}

static void ld16( lr35902_t *cpu ) {
    write_rp( cpu, read16( cpu, ( cpu->pc+1 ), true ), cpu->op.p );
    cpu->pc+=2;
}

static void li16( lr35902_t *cpu ) {
    write8( cpu, cpu->op.p < 2 ? read_rp( cpu, cpu->op.p ) : ( cpu->op.p == 2 ? cpu->hl++ : cpu->hl-- ), cpu->a, true );
}

static void add16( lr35902_t *cpu ) {
    uint16_t oldhl = cpu->hl;
    cpu->hl = cpu->hl + read_rp( cpu, cpu->op.p );
    SetFlags( cpu, cpu->fz, 0, ( cpu->hl & 0xfff ) < ( oldhl & 0xfff ), ( cpu->hl & 0xffff ) < ( oldhl & 0xffff ) );
}

static void la16( lr35902_t *cpu ) {
    cpu->a = read8( cpu, cpu->op.p < 2 ? read_rp( cpu, cpu->op.p ) : ( cpu->op.p == 2 ? cpu->hl++ : cpu->hl-- ), true );
}

static void jp( lr35902_t* cpu ) {
    uint16_t dest = read16( cpu, cpu->pc + 1, true );
    printf( "jump to %04x\n", dest );
    cpu->pc = dest - 1;
}

static void jr( lr35902_t* cpu ) {
    cpu->pc = cpu->pc + (int8_t)read8( cpu, cpu->pc + 1, true ) + 1;
}

static void jrx( lr35902_t* cpu ) {
    if ( NzNc( cpu ) )
        cpu->pc = cpu->pc + (int8_t)read8( cpu, cpu->pc + 1, true );
    cpu->pc++;
}

static void pushw( lr35902_t* cpu, uint16_t word ) {
    write8( cpu, cpu->sp--, ( word & 0xff00 ) >> 8, false );
    write8( cpu, cpu->sp--, ( word & 0xff ), false );
}

static uint16_t popw( lr35902_t* cpu ) {
    cpu->sp += 2;
    return ( read8( cpu, cpu->sp, false) << 8 ) + read8( cpu, cpu->sp - 1, false );
}

static void call( lr35902_t* cpu ) {
    pushw( cpu, cpu->pc + 3 );
    cpu->pc = read16( cpu, cpu->pc + 1, true ) - 1;
}

static void callx( lr35902_t* cpu ) {
    if ( NzNc( cpu ) ) {
        pushw( cpu, cpu->pc + 3 );
        cpu->pc = read16( cpu, cpu->pc + 1, true ) - 1;
    } else {
        cpu->pc += 2;
    }
}

static void jpx( lr35902_t* cpu ) {
    if ( NzNc( cpu ) ) {
        cpu->pc = read16( cpu, cpu->pc + 1, true ) - 1;
    } else {
        cpu->pc += 2;
    }
}

static void ret( lr35902_t* cpu ) {
    cpu->pc = popw( cpu ) - 1;
}

static void reti( lr35902_t* cpu ) {
    cpu->pc = popw( cpu ) - 1;
    cpu->ifl = 1;
}

static void retx( lr35902_t* cpu ) {
    if ( NzNc( cpu ) ) {
        cpu->pc = popw( cpu ) - 1;
    }
}

static void hlt( lr35902_t *cpu ) {
    cpu->halted = true;
}

static void rst( lr35902_t *cpu ) {
    pushw( cpu, cpu->pc + 2 );
    cpu->pc = cpu->op.y << 3;
}

static void pop( lr35902_t *cpu ) {
    write_rp2( cpu, popw( cpu ), cpu->op.p );
}

static void psh( lr35902_t *cpu ) {
    pushw( cpu, read_rp2( cpu, cpu->op.p ) );
}

static void edi( lr35902_t *cpu ) {
    cpu->ifl = cpu->op.q;
}

static void stop( lr35902_t *cpu ) {
    hlt( cpu );
}

static void e0( lr35902_t *cpu ) {
    write8( cpu, 0xFF00 + read8( cpu, ++cpu->pc, true ), cpu->a, true );
}

static void e2( lr35902_t *cpu ) {
    write8( cpu, 0xFF00 + cpu->c, cpu->a, true );
}

static void ea( lr35902_t *cpu ) {
    write8( cpu, read16( cpu, cpu->pc+1, true ), cpu->a, true );
    cpu->pc+=2;
}

static void f0( lr35902_t *cpu ) {
    cpu->a = read8( cpu, 0xFF00 + read8( cpu, ++cpu->pc, true ), true );
}

static void f2( lr35902_t *cpu ) {
    cpu->a = read8( cpu, 0xFF00 + cpu->c, true );
}

static void fa( lr35902_t *cpu ) {
    cpu->a = read8( cpu, read16( cpu, cpu->pc+1, true ), true );
    cpu->pc+=2;
}

static void addsp( lr35902_t *cpu ) {
    uint16_t oldsp = cpu->sp;
    uint8_t val = (int8_t)read8( cpu, cpu->pc++, true );
    cpu->sp = cpu->sp + val;
    if ( val < 0 )
        SetFlags( cpu, 0, 0, ( oldsp & 0xf ) > ( cpu->sp & 0xf ), ( oldsp & 0xff ) > ( cpu->sp & 0xff ) );
    else
        SetFlags( cpu, 0, 0, ( oldsp & 0xf ) < ( cpu->sp & 0xf ), ( oldsp & 0xff ) < ( cpu->sp & 0xff ) );
}

static void jphl( lr35902_t *cpu ) {
    cpu->pc = cpu->hl - 1;
}

static void lhlsi( lr35902_t *cpu ) {
    uint8_t val = (int8_t)read8( cpu, cpu->pc++, true );
    cpu->hl = cpu->sp + val;
    if ( val < 0 )
        SetFlags( cpu, 0, 0, ( cpu->sp & 0xf ) > ( cpu->hl & 0xf ), ( cpu->sp & 0xff ) > ( cpu->hl & 0xff ) );
    else
        SetFlags( cpu, 0, 0, ( cpu->sp & 0xf ) < ( cpu->hl & 0xf ), ( cpu->sp & 0xff ) < ( cpu->hl & 0xff ) );
}

static void l16sp( lr35902_t *cpu ) {
    cpu->sp = read16( cpu, cpu->pc+1, true );  
    cpu->pc += 2;
}

static void lsphl( lr35902_t *cpu ) {
    cpu->sp = cpu->hl;
}

static void rlc( lr35902_t *cpu ) {
    uint8_t in = read_r( cpu, cpu->op.z ), newc = ( cpu->a & 0x80 ) >> 7, out = ( in << 1 ) | newc;
    write_r( cpu, out, cpu->op.z );
    SetFlags( cpu, out == 0, 0, 0, newc );
}

static void rrc( lr35902_t *cpu ) {
    uint8_t in = read_r( cpu, cpu->op.z ), newc = in & 0x1, out = ( in >> 1 ) | ( newc << 7 );
    SetFlags( cpu, out == 0, 0, 0, newc );
    write_r( cpu, out, cpu->op.z );
}

static void rl( lr35902_t *cpu ) {
    uint8_t in = read_r( cpu, cpu->op.z ), newc = ( cpu->a & 0x80 ) >> 7, out = ( in << 1 ) | cpu->fc;
    write_r( cpu, out, cpu->op.z );
    SetFlags( cpu, out == 0, 0, 0, newc );
}

static void rr( lr35902_t *cpu ) {
    uint8_t in = read_r( cpu, cpu->op.z ), newc = in & 0x1, out = ( in >> 1 ) | ( cpu->fc << 7 );
    write_r( cpu, out, cpu->op.z );  
    SetFlags( cpu, out == 0, 0, 0, newc );
}

static void sla( lr35902_t *cpu ) {
    uint8_t in = read_r( cpu, cpu->op.z ), newc = ( in & 0x8 ) >> 7, out = ( in << 1 );
    write_r( cpu, out, cpu->op.z );
    SetFlags( cpu, out == 0, 0, 0, newc );
}

static void sra( lr35902_t *cpu ) {
    uint8_t in = read_r( cpu, cpu->op.z ), newc = in & 0x1, out = ( in >> 1 ) | ( ( in & 0x80 ) );
    write_r( cpu, out, cpu->op.z );
    SetFlags( cpu, cpu->a == 0, 0, 0, newc );
}

static void swap( lr35902_t *cpu ) {
    uint8_t in = read_r( cpu, cpu->op.z ), upper = in >> 4, lower = in & 0x0F, out = ( lower << 4 ) | upper;
    write_r( cpu, out, cpu->op.z );
    SetFlags( cpu, out == 0, 0, 0, 0 );
}

static void srl( lr35902_t *cpu ) {
    uint8_t in = read_r( cpu, cpu->op.z ), newc = in & 0x1, out = ( in >> 1 );
    write_r( cpu, out, cpu->op.z );
    SetFlags( cpu, cpu->a == 0, 0, 0, newc );
}

static void bit( lr35902_t *cpu ) {
    printf( "bit unimplemented\n" );
    exit( 1 );
}

static void res( lr35902_t *cpu ) {
    printf( "res unimplemented\n" );
    exit( 1 );
}

static void set( lr35902_t *cpu ) {
    printf( "set unimplemented\n" );
    exit( 1 );
}

void (*cbops[32])( lr35902_t* cpu ) = {
    rlc, rrc, rl, rr, sla, sra, swap, srl,
    bit, bit, bit, bit, bit, bit, bit, bit,
    res, res, res, res, res, res, res, res,
    set, set, set, set, set, set, set, set,
};

static void cb( lr35902_t *cpu ) {
    cpu->pc++;
    cpu->op.full = read8( cpu, cpu->pc, true );
    cbops[( cpu->op.full & 0xF8 ) >> 3]( cpu );
}

static void bad( lr35902_t *cpu ) {
    printf( "bad instruction %02x\n", cpu->op.full );
    exit( 1 );
}

void (*ops[256])( lr35902_t* cpu ) = {
    nop,  ld16, li16, in16,  inc,  dec,  ldd8, rlca,    l16sp,add16,la16, de16,   inc,  dec,  ldd8, rrca, 
    stop, ld16, li16, in16,  inc,  dec,  ldd8, rla,     jr,   add16,la16, de16,   inc,  dec,  ldd8, rra, 
    jrx,  ld16, li16, in16,  inc,  dec,  ldd8, daa,     jrx,  add16,la16, de16,   inc,  dec,  ldd8, cpl, 
    jrx,  ld16, li16, in16,  inc,  dec,  ldd8, scf,     jrx,  add16,la16, de16,   inc,  dec,  ldd8, ccf, 

    ld,   ld,   ld,   ld,     ld,   ld,   ld,   ld,     ld,   ld,   ld,   ld,     ld,   ld,   ld,   ld,  
    ld,   ld,   ld,   ld,     ld,   ld,   ld,   ld,     ld,   ld,   ld,   ld,     ld,   ld,   ld,   ld,  
    ld,   ld,   ld,   ld,     ld,   ld,   ld,   ld,     ld,   ld,   ld,   ld,     ld,   ld,   ld,   ld,  
    ld,   ld,   ld,   ld,     ld,   ld,   hlt,  ld,     ld,   ld,   ld,   ld,     ld,   ld,   ld,   ld,  

    add,  add,  add,  add,    add,  add,  add,  add,    adc,  adc,  adc,  adc,    adc,  adc,  adc,  adc, 
    sub,  sub,  sub,  sub,    sub,  sub,  sub,  sub,    sbc,  sbc,  sbc,  sbc,    sbc,  sbc,  sbc,  sbc, 
    and,  and,  and,  and,    and,  and,  and,  and,    xor,  xor,  xor,  xor,    xor,  xor,  xor,  xor, 
    or,   or,   or,   or,     or,   or,   or,   or,     cp,   cp,   cp,   cp,     cp,   cp,   cp,   cp,  

    retx, pop,  jpx,  jp,     callx,psh,  add,  rst,    retx, ret,  jpx,  cb,     callx,call, adc,  rst, 
    retx, pop,  jpx,  bad,    callx,psh,  sub,  rst,    retx, reti, jpx,  bad,    callx,bad,  sbc,  rst, 
    e0,   pop,  e2,   bad,    bad,  psh,  and,  rst,    addsp,jphl, ea,   bad,    bad,  bad,  xor,  rst, 
    f0,   pop,  f2,   edi,    bad,  psh,  or,   rst,    lhlsi,lsphl,fa,   edi,    bad,  bad,  cp,   rst, 
};

static void Step( lr35902_t *cpu ) {
    if ( cpu->halted )
        return;
    cpu->op.full = cpu->bus->Read8( cpu->bus, cpu->pc, false );
    uint16_t imm16 = read16(cpu, cpu->pc+1, false);
    printf("[%04x] af:%04x bc:%04x de: %04x hl: %04x sp: %04x op: %02x imm16: %02x\n", cpu->pc, cpu->af, cpu->bc, cpu->de, cpu->hl, cpu->sp, cpu->op.full, imm16 );

    ops[cpu->op.full]( cpu );
    cpu->pc++;
}

static void Reset( lr35902_t *cpu ) {
    cpu->af = 0x01b0;
    cpu->bc = 0x0013;
    cpu->de = 0x00d8;
    cpu->hl = 0x014d;
    cpu->sp = 0xfffe;
    cpu->pc = 0x0100;
    cpu->halted = false;
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