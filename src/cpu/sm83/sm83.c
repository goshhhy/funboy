/* please dont use this code as a "good example" of anything 
 * most people will probably hate it, i know */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../../device/device.h"
#include "sm83.h"

#define write8( cpu, addr, val, final ) cpu->bus->Write8( cpu->bus, addr, val, final )
#define read8( cpu, addr, final ) cpu->bus->Read8( cpu->bus, addr, final )
#define read16( cpu, addr, final ) ( ( cpu->bus->Read8( cpu->bus, addr + 1, false ) << 8 ) + cpu->bus->Read8( cpu->bus, addr, final ) )

extern bool printSteps;

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

static void nop( sm83_t *cpu ) {
}

void SetFlags( sm83_t *cpu, uint8_t z, uint8_t n, uint8_t h, uint8_t c ) {
    cpu->fz = z;
    cpu->fn = n;
    cpu->fh = h;
    cpu->fc = c;
}

bool NzNc( sm83_t *cpu ) {
    bool cond = ( ( ( cpu->op.p & 1 ) == 0 ) ? cpu->fz : cpu->fc );
    return ( cpu->op.q == 0 ) ? !cond : cond;
}

// REGISTER READ/WRITES

static uint8_t read_r( sm83_t *cpu, uint8_t r ) {
    uint8_t *ri[8] = {&cpu->b, &cpu->c,  &cpu->d,  &cpu->e,  &cpu->h,  &cpu->l, NULL,  &cpu->a };
    return r == LD_MODE_HL ? read8( cpu, cpu->hl, true ) : *ri[r];
}

static void write_r( sm83_t *cpu, uint8_t val, uint8_t r ) {
    uint8_t *ri[8] = {&cpu->b, &cpu->c,  &cpu->d,  &cpu->e,  &cpu->h,  &cpu->l, NULL,  &cpu->a };
    if ( r == LD_MODE_HL )
        write8( cpu, cpu->hl, val, true );
    else
        *ri[r] = val;
}

static uint16_t read_rp( sm83_t *cpu, uint8_t r ) {
    uint16_t *ri[4] = {&cpu->bc, &cpu->de,  &cpu->hl,  &cpu->sp };
    return *ri[r];
}

static void write_rp( sm83_t *cpu, uint16_t val, uint8_t r ) {
    uint16_t *ri[4] = {&cpu->bc, &cpu->de,  &cpu->hl,  &cpu->sp };
    *ri[r] = val;
}

static uint16_t read_rp2( sm83_t *cpu, uint8_t r ) {
    uint16_t *ri[4] = {&cpu->bc, &cpu->de,  &cpu->hl,  &cpu->af };
    return *ri[r];
}

static void write_rp2( sm83_t *cpu, uint16_t val, uint8_t r ) {
    uint16_t *ri[4] = {&cpu->bc, &cpu->de,  &cpu->hl, &cpu->af };
    *ri[r] = val & ( r == 3 ? 0xFFF0 : 0xFFFF );
}

// OPCODES

static void ld( sm83_t *cpu ) {
    write_r( cpu, read_r( cpu, cpu->op.z ), cpu->op.y );
}

static void ldd8( sm83_t *cpu ) {
    write_r( cpu, read8( cpu, cpu->pc++ + 1, false ), cpu->op.y );
}

static void add( sm83_t *cpu ) {
    uint8_t other = ( cpu->op.x == 3 ) ? read8( cpu, ++cpu->pc, true ) : read_r( cpu, cpu->op.z );
    uint8_t orig = cpu->a, res = cpu->a = cpu->a + other;
    SetFlags( cpu, res == 0, 0, ( res & 0xf ) < ( orig & 0xf ), res < orig );
}

static void adc( sm83_t *cpu ) {
    uint8_t other = ( cpu->op.x == 3 ) ? read8( cpu, ++cpu->pc, true ) : read_r( cpu, cpu->op.z );
    uint8_t orig = cpu->a, res = cpu->a = cpu->a + other + cpu->fc;
    SetFlags( cpu, res == 0, 0, ( other & 0xf ) + ( orig & 0xf ) + cpu->fc > 0xf, ((uint16_t)orig + (uint16_t)other + (uint16_t)cpu->fc) > 0xFF);
}

static void sub( sm83_t *cpu ) {
    uint8_t other = ( cpu->op.x == 3 ) ? read8( cpu, ++cpu->pc, true ) : read_r( cpu, cpu->op.z );
    uint8_t orig = cpu->a, res = cpu->a = cpu->a - other;
    SetFlags( cpu, res == 0, 1, ( res & 0xf ) > ( orig & 0xf ), res > orig );
}

static void sbc( sm83_t *cpu ) {
    uint8_t other = ( cpu->op.x == 3 ) ? read8( cpu, ++cpu->pc, true ) : read_r( cpu, cpu->op.z );
    uint8_t orig = cpu->a, res = cpu->a = ( cpu->a - other ) - cpu->fc;
    SetFlags( cpu, res == 0, 1, (( other & 0xf ) + cpu->fc ) > ( orig & 0xf ), ((orig - other) - cpu->fc) < 0 );
}

static void and( sm83_t *cpu ) {
    uint8_t other = ( cpu->op.x == 3 ) ? read8( cpu, ++cpu->pc, true ) : read_r( cpu, cpu->op.z );
    SetFlags( cpu, ( ( cpu->a = ( cpu->a & other ) ) == 0 ), 0, 1, 0 );
}

static void xor( sm83_t *cpu ) {
    uint8_t other = ( cpu->op.x == 3 ) ? read8( cpu, ++cpu->pc, true ) : read_r( cpu, cpu->op.z );
    SetFlags( cpu, ( ( cpu->a = ( cpu->a ^ other ) ) == 0 ), 0, 0, 0 );
}

static void or( sm83_t *cpu ) {
    uint8_t other = ( cpu->op.x == 3 ) ? read8( cpu, ++cpu->pc, true ) : read_r( cpu, cpu->op.z );
    SetFlags( cpu, ( ( cpu->a = ( cpu->a | other ) ) == 0 ), 0, 0, 0 );
}

static void cp( sm83_t *cpu ) {
    uint8_t other = ( cpu->op.x == 3 ) ? read8( cpu, ++cpu->pc, true ) : read_r( cpu, cpu->op.z );
    uint8_t res = cpu->a - other;
    SetFlags( cpu, ( res == 0 ), 1, ( res & 0x0f ) > ( cpu->a & 0x0f ), ( res ) > ( cpu->a ) );
}

static void cpl( sm83_t *cpu ) {
    cpu->a = ~cpu->a;
    SetFlags( cpu, cpu->fz, 1, 1, cpu->fc );
}

static void inc( sm83_t *cpu ) {
    uint8_t in = read_r( cpu, cpu->op.y ), res = in + 1;
    SetFlags( cpu, res == 0, 0, ( res & 0xf ) < ( in & 0xf), cpu->fc );
    write_r( cpu, res, cpu->op.y );
}

static void dec( sm83_t *cpu ) {
    uint8_t in = read_r( cpu, cpu->op.y ), res = in - 1;
    SetFlags( cpu, res == 0, 1, ( res & 0x0f ) > ( in & 0x0f), cpu->fc );
    write_r( cpu, res, cpu->op.y );
}

static void ccf( sm83_t *cpu ) {
    SetFlags( cpu, cpu->fz, 0, 0, cpu->fc ^ 1 );
}

static void scf( sm83_t *cpu ) {
    SetFlags( cpu, cpu->fz, 0, 0, 1 );
}

static void daa( sm83_t *cpu ) {
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

static void rlca( sm83_t *cpu ) {
    SetFlags( cpu, 0, 0, 0, ( cpu->a & 0x80 ) >> 7 );
    cpu->a = ( cpu->a << 1 ) + cpu->fc;
}

static void rla( sm83_t *cpu ) {
    uint8_t newc = ( cpu->a & 0x80 ) >> 7;
    cpu->a = ( cpu->a << 1 ) + cpu->fc;
    SetFlags( cpu, 0, 0, 0, newc );
}

static void rrca( sm83_t *cpu ) {
    SetFlags( cpu, 0, 0, 0, ( cpu->a & 0x01 ) );
    cpu->a = ( cpu->a >> 1 ) | ( ( cpu->a & 0x01 ) << 7 );
}

static void rra( sm83_t *cpu ) {
    uint8_t newc = ( cpu->a & 0x01 );
    cpu->a = ( cpu->a >> 1 ) + ( cpu->fc << 7 );
    SetFlags( cpu, 0, 0, 0, newc );
}

static void in16( sm83_t *cpu ) {
    write_rp( cpu, read_rp( cpu, cpu->op.p ) + 1, cpu->op.p );
}

static void de16( sm83_t *cpu ) {
    write_rp( cpu, read_rp( cpu, cpu->op.p ) - 1, cpu->op.p );
}

static void ld16( sm83_t *cpu ) {
    write_rp( cpu, read16( cpu, ( cpu->pc+1 ), true ), cpu->op.p );
    cpu->pc+=2;
}

static void li16( sm83_t *cpu ) {
    write8( cpu, cpu->op.p < 2 ? read_rp( cpu, cpu->op.p ) : ( cpu->op.p == 2 ? cpu->hl++ : cpu->hl-- ), cpu->a, true );
}

static void add16( sm83_t *cpu ) {
    uint16_t oldhl = cpu->hl;
    cpu->hl = cpu->hl + read_rp( cpu, cpu->op.p );
    SetFlags( cpu, cpu->fz, 0, ( cpu->hl & 0xfff ) < ( oldhl & 0xfff ), ( cpu->hl & 0xffff ) < ( oldhl & 0xffff ) );
}

static void la16( sm83_t *cpu ) {
    cpu->a = read8( cpu, cpu->op.p < 2 ? read_rp( cpu, cpu->op.p ) : ( cpu->op.p == 2 ? cpu->hl++ : cpu->hl-- ), true );
}

static void jp( sm83_t* cpu ) {
    uint16_t dest = read16( cpu, cpu->pc + 1, true );
    cpu->pc = dest - 1;
}

static void jr( sm83_t* cpu ) {
    cpu->pc = cpu->pc + (int8_t)read8( cpu, cpu->pc + 1, true ) + 1;
}

static void jrx( sm83_t* cpu ) {
    if ( NzNc( cpu ) ) {
        cpu->pc = cpu->pc + (int8_t)read8( cpu, cpu->pc + 1, true );
        cpu->timetarget += 4;
    }
    cpu->pc++;
}

static void pushw( sm83_t* cpu, uint16_t word ) {
    cpu->sp -= 2;
    write8( cpu, cpu->sp + 1, ( word & 0xff00 ) >> 8, false );
    write8( cpu, cpu->sp, ( word & 0xff ), true );
}

static uint16_t popw( sm83_t* cpu ) {
    cpu->sp += 2;
    return ( read8( cpu, cpu->sp - 1, false) << 8 ) + read8( cpu, cpu->sp - 2, true );
}

static void call( sm83_t* cpu ) {
    pushw( cpu, cpu->pc + 3 );
    cpu->pc = read16( cpu, cpu->pc + 1, true ) - 1;
}

static void callx( sm83_t* cpu ) {
    if ( NzNc( cpu ) ) {
        pushw( cpu, cpu->pc + 3 );
        cpu->pc = read16( cpu, cpu->pc + 1, true ) - 1;
        cpu->timetarget += 12;
    } else {
        cpu->pc += 2;
    }
}

static void jpx( sm83_t* cpu ) {
    if ( NzNc( cpu ) ) {
        cpu->pc = read16( cpu, cpu->pc + 1, true ) - 1;
        cpu->timetarget += 4;
    } else {
        cpu->pc += 2;
    }
}

static void ret( sm83_t* cpu ) {
    cpu->pc = popw( cpu ) - 1;
}

static void reti( sm83_t* cpu ) {
    cpu->pc = popw( cpu ) - 1;
    cpu->ifl = 1;
}

static void retx( sm83_t* cpu ) {
    if ( NzNc( cpu ) ) {
        cpu->pc = popw( cpu ) - 1;
        cpu->timetarget += 12;
    }
}

static void hlt( sm83_t *cpu ) {
    cpu->halted = true;
}

static void rst( sm83_t *cpu ) {
    pushw( cpu, cpu->pc + 1 );
    cpu->pc = (cpu->op.y << 3) - 1;
}

static void pop( sm83_t *cpu ) {
    write_rp2( cpu, popw( cpu ), cpu->op.p );
}

static void psh( sm83_t *cpu ) {
    pushw( cpu, read_rp2( cpu, cpu->op.p ) );
}

static void edi( sm83_t *cpu ) {
    cpu->ifl = cpu->op.q;
}

static void stop( sm83_t *cpu ) {
    hlt( cpu );
}

static void e0( sm83_t *cpu ) {
    write8( cpu, 0xFF00 + read8( cpu, ++cpu->pc, true ), cpu->a, true );
}

static void e2( sm83_t *cpu ) {
    write8( cpu, 0xFF00 + cpu->c, cpu->a, true );
}

static void ea( sm83_t *cpu ) {
    write8( cpu, read16( cpu, cpu->pc+1, true ), cpu->a, true );
    cpu->pc+=2;
}

static void f0( sm83_t *cpu ) {
    cpu->a = read8( cpu, 0xFF00 + read8( cpu, ++cpu->pc, true ), true );
}

static void f2( sm83_t *cpu ) {
    cpu->a = read8( cpu, 0xFF00 + cpu->c, true );
}

static void fa( sm83_t *cpu ) {
    cpu->a = read8( cpu, read16( cpu, cpu->pc+1, true ), true );
    cpu->pc+=2;
}

static void addsp( sm83_t *cpu ) {
    uint16_t oldsp = cpu->sp;
    int8_t val = (int8_t)read8( cpu, ++cpu->pc, true );
    cpu->sp = cpu->sp + val;
    SetFlags( cpu, 0, 0, ( oldsp & 0xf ) > ( cpu->sp & 0xf ), ( oldsp & 0xff ) > ( cpu->sp & 0xff ) );
}

static void jphl( sm83_t *cpu ) {
    cpu->pc = cpu->hl - 1;
}

static void lhlsi( sm83_t *cpu ) {
    int8_t val = (int8_t)read8( cpu, ++cpu->pc, true );
    printf("e8 is %hhi\n", val );
    cpu->hl = (int16_t)cpu->sp + val;
    SetFlags( cpu, 0, 0, ( cpu->hl & 0xf ) < ( cpu->sp & 0xf ), ( cpu->hl & 0xff ) < ( cpu->sp & 0xff ) );
}

static void l16sp( sm83_t *cpu ) {
    uint16_t addr = read16( cpu, cpu->pc+1, true );  
    write8( cpu, addr + 1, (cpu->sp & 0xff00) >> 8, false );
    write8( cpu, addr, cpu->sp & 0xff, true );
    cpu->pc += 2;
}

static void lsphl( sm83_t *cpu ) {
    cpu->sp = cpu->hl;
}

static void rlc( sm83_t *cpu ) {
    uint8_t in = read_r( cpu, cpu->op.z ), newc = ( in & 0x80 ) >> 7, out = ( in << 1 ) | newc;
    write_r( cpu, out, cpu->op.z );
    SetFlags( cpu, out == 0, 0, 0, newc );
}

static void rrc( sm83_t *cpu ) {
    uint8_t in = read_r( cpu, cpu->op.z ), newc = in & 0x1, out = ( in >> 1 ) | ( newc << 7 );
    SetFlags( cpu, out == 0, 0, 0, newc );
    write_r( cpu, out, cpu->op.z );
}

static void rl( sm83_t *cpu ) {
    uint8_t in = read_r( cpu, cpu->op.z ), newc = ( in & 0x80 ) >> 7, out = ( in << 1 ) | cpu->fc;
    write_r( cpu, out, cpu->op.z );
    SetFlags( cpu, out == 0, 0, 0, newc );
}

static void rr( sm83_t *cpu ) {
    uint8_t in = read_r( cpu, cpu->op.z ), newc = in & 0x1, out = ( in >> 1 ) | ( cpu->fc << 7 );
    write_r( cpu, out, cpu->op.z );  
    SetFlags( cpu, out == 0, 0, 0, newc );
}

static void sla( sm83_t *cpu ) {
    uint8_t in = read_r( cpu, cpu->op.z ), newc = ( in & 0x80 ) >> 7, out = ( in << 1 ) & 0xFE;
    write_r( cpu, out, cpu->op.z );
    SetFlags( cpu, out == 0, 0, 0, newc );
}

static void sra( sm83_t *cpu ) {
    uint8_t in = read_r( cpu, cpu->op.z ), newc = in & 0x1, out = ( in >> 1 ) | ( ( in & 0x80 ) );
    write_r( cpu, out, cpu->op.z );
    SetFlags( cpu, out == 0, 0, 0, newc );
}

static void swap( sm83_t *cpu ) {
    uint8_t in = read_r( cpu, cpu->op.z ), upper = in >> 4, lower = in & 0x0F, out = ( lower << 4 ) | upper;
    write_r( cpu, out, cpu->op.z );
    SetFlags( cpu, out == 0, 0, 0, 0 );
}

static void srl( sm83_t *cpu ) {
    uint8_t in = read_r( cpu, cpu->op.z ), newc = in & 0x1, out = ( in >> 1 ) & 0x7F;
    write_r( cpu, out, cpu->op.z );
    SetFlags( cpu, out == 0, 0, 0, newc );
}

static void bit( sm83_t *cpu ) {
    SetFlags( cpu, ( read_r( cpu, cpu->op.z ) & ( 0x01 << cpu->op.y ) ) == 0 , 0, 1, cpu->fc );
}

static void res( sm83_t *cpu ) {
    write_r( cpu, read_r( cpu, cpu->op.z ) & ~( 0x01 << cpu->op.y ), cpu->op.z );
}

static void set( sm83_t *cpu ) {
    write_r( cpu, read_r( cpu, cpu->op.z ) | ( 0x01 << cpu->op.y ), cpu->op.z );
}

void (*cbops[32])( sm83_t* cpu ) = {
    rlc, rrc, rl, rr, sla, sra, swap, srl,
    bit, bit, bit, bit, bit, bit, bit, bit,
    res, res, res, res, res, res, res, res,
    set, set, set, set, set, set, set, set,
};

static void cb( sm83_t *cpu ) {
    cpu->pc++;
    cpu->op.full = read8( cpu, cpu->pc, true );
    cbops[( cpu->op.full & 0xF8 ) >> 3]( cpu );
}

static void bad( sm83_t *cpu ) {
    printf( "bad instruction %02x\n", cpu->op.full );
    exit( 1 );
}

void (*ops[256])( sm83_t* cpu ) = {
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

int timings[256] = {
    4, 12,  8,  8,  4,  4,  8,  4, 20,  8,  8,  8,  4,  4,  8,  4,
    4, 12,  8,  8,  4,  4,  8,  4, 12,  8,  8,  8,  4,  4,  8,  4,
    8, 12,  8,  8,  4,  4,  8,  4,  8,  8,  8,  8,  4,  4,  8,  4,
    8, 12,  8,  8, 12, 12, 12,  4,  8,  8,  8,  8,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    8,  8,  8,  8,  8,  8,  4,  8,  4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    8, 12, 12, 16, 12, 16,  8, 16,  8, 16, 12,  4, 12, 24,  8,  16,
    8, 12, 12,  4, 12, 16,  8, 16,  8, 16, 12,  4, 12,  4,  8,  16,
    12, 12, 8,  4,  4, 16,  8, 16, 16,  4, 16,  4,  4,  4,  8,  16,
    12, 12, 8,  4,  4, 16,  8, 16, 12,  8, 16,  4,  4,  4,  8,  16,
};

static void Step( sm83_t *cpu ) {
    uint8_t active;

    if ( cpu->fetched == false ) {
        cpu->op.full = cpu->bus->Read8( cpu->bus, cpu->pc, false );
        cpu->timetarget += timings[cpu->op.full];
        cpu->fetched = true;
    }

    cpu->timetarget--;
    if ( cpu->timetarget > 0 )
        return;

    if ( ! cpu->halted ) {
        uint16_t imm16 = read16(cpu, cpu->pc+1, false);
        printf("[%04x] af:%04x bc:%04x de: %04x hl: %04x sp: %04x op: %02x imm16: %02x\n", cpu->pc, cpu->af, cpu->bc, cpu->de, cpu->hl, cpu->sp, cpu->op.full, imm16 );

        ops[cpu->op.full]( cpu );
        cpu->pc++;
    }

    // run interrupts if applicable
    if ( ( cpu->ifl == 1 ) || ( cpu->halted ) ) {
        active = ( read8( cpu, 0xffff, false ) & read8( cpu, 0xff0f, false ) ) & 0x1F; 
        if ( active ) {
            printf("running interrupt\n");
            pushw( cpu, cpu->pc );
            cpu->halted = false;
            cpu->ifl = 0;
            if ( active & 0x1 ) {
                write8( cpu, 0xff0f, active & 0x1E, true );
                cpu->pc = 0x40;
            } else if ( active & 0x2 ) {
                write8( cpu, 0xff0f, active & 0x1D, true );
                cpu->pc = 0x48;
            } else if ( active & 0x4 ) {
                write8( cpu, 0xff0f, active & 0x1B, true );
                cpu->pc = 0x50;
            } else if ( active & 0x8 ) {
                write8( cpu, 0xff0f, active & 0x17, true );
                cpu->pc = 0x58;
            } else if ( active & 0x10 ) {
                write8( cpu, 0xff0f, active & 0x0F, true );
                cpu->pc = 0x60;
            }
        }
    }

    cpu->fetched = false;
}

static void Interrupt( sm83_t *cpu, uint8_t inum ) {
        //fprintf( stderr, "interrupt %hhi set with interrupts %i, ie %hhi\n", inum, cpu->ifl, read8( cpu, 0xffff, false ) );
        write8( cpu, 0xff0f, read8( cpu, 0xff0f, false ) | (0x1 << inum), false );
        cpu->halted = false;
}

static void Reset( sm83_t *cpu ) {
    cpu->af = 0x01b0;
    cpu->bc = 0x0013;
    cpu->de = 0x00d8;
    cpu->hl = 0x014d;
    cpu->sp = 0xfffe;
    cpu->pc = 0x0100;
    cpu->halted = false;
    cpu->fetched = false;
    cpu->timetarget = 1;
}

sm83_t *Sm83( busDevice_t *bus ) {
	sm83_t *cpu = calloc( 1, sizeof( sm83_t ) );
	cpu->bus = bus;
	cpu->Reset = Reset;
	cpu->Step = Step;
    cpu->Interrupt = Interrupt;
	Reset( cpu );
    printf( "sm83 cpu created\n" );
	return cpu;
}