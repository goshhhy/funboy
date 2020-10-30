#include <stdio.h>
#include <stdlib.h>

#include "../../device/device.h"

#include "sm83.h"
#include "sm83_ops.h"

/* SUPPORT */

#define write8( cpu, addr, val, final ) cpu->bus->Write8( cpu->bus, addr, val, final )
#define read8( cpu, addr, final ) cpu->bus->Read8( cpu->bus, addr, final )
#define read16( cpu, addr, final ) ( ( cpu->bus->Read8( cpu->bus, addr + 1, 0 ) << 8 ) + cpu->bus->Read8( cpu->bus, addr, final ) )

#define READ_HL ( ( cpu->h << 8 ) + cpu->l )
#define SET_HL(val) cpu->h = ( (val) >> 8); cpu->l = ((val) & 0xff)

void SetFlags( sm83_t *cpu, unsigned char z, unsigned char n, unsigned char h, unsigned char c ) {
    cpu->f.flags.fz = z;
    cpu->f.flags.fn = n;
    cpu->f.flags.fh = h;
    cpu->f.flags.fc = c;
}

int NzNc( sm83_t *cpu ) {
    int cond = ( ( ( cpu->op.pqxz.p & 1 ) == 0 ) ? cpu->f.flags.fz : cpu->f.flags.fc );
    return ( cpu->op.pqxz.q == 0 ) ? !cond : cond;
}

/* OPCODES */

void op_nop( sm83_t *cpu ) {
}

void op_ld( sm83_t *cpu ) {
    write_r( cpu, read_r( cpu, cpu->op.xyz.z ), cpu->op.xyz.y );
}

void op_ldd8( sm83_t *cpu ) {
    write_r( cpu, read8( cpu, cpu->pc++ + 1, 0 ), cpu->op.xyz.y );
}

void op_add( sm83_t *cpu ) {
    unsigned char other = ( cpu->op.xyz.x == 3 ) ? read8( cpu, ++cpu->pc, 1 ) : read_r( cpu, cpu->op.xyz.z );
    unsigned char orig = cpu->a, res = cpu->a = cpu->a + other;
    SetFlags( cpu, res == 0, 0, ( res & 0xf ) < ( orig & 0xf ), res < orig );
}

void op_adc( sm83_t *cpu ) {
    unsigned char other = ( cpu->op.xyz.x == 3 ) ? read8( cpu, ++cpu->pc, 1 ) : read_r( cpu, cpu->op.xyz.z );
    unsigned char orig = cpu->a, res = cpu->a = cpu->a + other + cpu->f.flags.fc;
    SetFlags( cpu, res == 0, 0, ( other & 0xf ) + ( orig & 0xf ) + cpu->f.flags.fc > 0xf, ((unsigned long)orig + (unsigned long)other + (unsigned long)cpu->f.flags.fc) > 0xFF);
}

void op_sub( sm83_t *cpu ) {
    unsigned char other = ( cpu->op.xyz.x == 3 ) ? read8( cpu, ++cpu->pc, 1 ) : read_r( cpu, cpu->op.xyz.z );
    unsigned char orig = cpu->a, res = cpu->a = cpu->a - other;
    SetFlags( cpu, res == 0, 1, ( res & 0xf ) > ( orig & 0xf ), res > orig );
}

void op_sbc( sm83_t *cpu ) {
    unsigned char other = ( cpu->op.xyz.x == 3 ) ? read8( cpu, ++cpu->pc, 1 ) : read_r( cpu, cpu->op.xyz.z );
    unsigned char orig = cpu->a, res = cpu->a = ( cpu->a - other ) - cpu->f.flags.fc;
    SetFlags( cpu, res == 0, 1, (( other & 0xf ) + cpu->f.flags.fc ) > ( orig & 0xf ), ((orig - other) - cpu->f.flags.fc) < 0 );
}

void op_and( sm83_t *cpu ) {
    unsigned char other = ( cpu->op.xyz.x == 3 ) ? read8( cpu, ++cpu->pc, 1 ) : read_r( cpu, cpu->op.xyz.z );
    SetFlags( cpu, ( ( cpu->a = ( cpu->a & other ) ) == 0 ), 0, 1, 0 );
}

void op_xor( sm83_t *cpu ) {
    unsigned char other = ( cpu->op.xyz.x == 3 ) ? read8( cpu, ++cpu->pc, 1 ) : read_r( cpu, cpu->op.xyz.z );
    SetFlags( cpu, ( ( cpu->a = ( cpu->a ^ other ) ) == 0 ), 0, 0, 0 );
}

void op_or( sm83_t *cpu ) {
    unsigned char other = ( cpu->op.xyz.x == 3 ) ? read8( cpu, ++cpu->pc, 1 ) : read_r( cpu, cpu->op.xyz.z );
    SetFlags( cpu, ( ( cpu->a = ( cpu->a | other ) ) == 0 ), 0, 0, 0 );
}

void op_cp( sm83_t *cpu ) {
    unsigned char other = ( cpu->op.xyz.x == 3 ) ? read8( cpu, ++cpu->pc, 1 ) : read_r( cpu, cpu->op.xyz.z );
    unsigned char res = cpu->a - other;
    SetFlags( cpu, ( res == 0 ), 1, ( res & 0x0f ) > ( cpu->a & 0x0f ), ( res ) > ( cpu->a ) );
}

void op_cpl( sm83_t *cpu ) {
    cpu->a = ~cpu->a;
    SetFlags( cpu, cpu->f.flags.fz, 1, 1, cpu->f.flags.fc );
}

void op_inc( sm83_t *cpu ) {
    unsigned char in = read_r( cpu, cpu->op.xyz.y ), res = in + 1;
    SetFlags( cpu, res == 0, 0, ( res & 0xf ) < ( in & 0xf), cpu->f.flags.fc );
    write_r( cpu, res, cpu->op.xyz.y );
}

void op_dec( sm83_t *cpu ) {
    unsigned char in = read_r( cpu, cpu->op.xyz.y ), res = in - 1;
    SetFlags( cpu, res == 0, 1, ( res & 0x0f ) > ( in & 0x0f), cpu->f.flags.fc );
    write_r( cpu, res, cpu->op.xyz.y );
}

void op_ccf( sm83_t *cpu ) {
    SetFlags( cpu, cpu->f.flags.fz, 0, 0, cpu->f.flags.fc ^ 1 );
}

void op_scf( sm83_t *cpu ) {
    SetFlags( cpu, cpu->f.flags.fz, 0, 0, 1 );
}

void op_daa( sm83_t *cpu ) {
    int carry = cpu->f.flags.fc;
    if ( cpu->f.flags.fn ) {
        cpu->a = ( cpu->a - ( cpu->f.flags.fc ? 0x60 : 0 ) ) - ( cpu->f.flags.fh ? 0x6 : 0 );
    } else {
        if ( cpu->f.flags.fc || ( cpu->a > 0x99 ) ) {
            cpu->a += 0x60;
            carry = 1;
        }
        if ( cpu->f.flags.fh || ( ( cpu->a & 0xf ) > 0x9 ) ) {
            cpu->a += 0x6;
        }
    }
    SetFlags( cpu, cpu->a == 0, cpu->f.flags.fn, 0, carry );
}

void op_rlca( sm83_t *cpu ) {
    SetFlags( cpu, 0, 0, 0, ( cpu->a & 0x80 ) >> 7 );
    cpu->a = ( cpu->a << 1 ) + cpu->f.flags.fc;
}

void op_rla( sm83_t *cpu ) {
    unsigned char newc = ( cpu->a & 0x80 ) >> 7;
    cpu->a = ( cpu->a << 1 ) + cpu->f.flags.fc;
    SetFlags( cpu, 0, 0, 0, newc );
}

void op_rrca( sm83_t *cpu ) {
    SetFlags( cpu, 0, 0, 0, ( cpu->a & 0x01 ) );
    cpu->a = ( cpu->a >> 1 ) | ( ( cpu->a & 0x01 ) << 7 );
}

void op_rra( sm83_t *cpu ) {
    unsigned char newc = ( cpu->a & 0x01 );
    cpu->a = ( cpu->a >> 1 ) + ( cpu->f.flags.fc << 7 );
    SetFlags( cpu, 0, 0, 0, newc );
}

void op_in16( sm83_t *cpu ) {
    write_rp( cpu, read_rp( cpu, cpu->op.pqxz.p ) + 1, cpu->op.pqxz.p );
}

void op_de16( sm83_t *cpu ) {
    write_rp( cpu, read_rp( cpu, cpu->op.pqxz.p ) - 1, cpu->op.pqxz.p );
}

void op_ld16( sm83_t *cpu ) {
    write_rp( cpu, read16( cpu, ( cpu->pc+1 ), 1 ), cpu->op.pqxz.p );
    cpu->pc+=2;
}

void op_li16( sm83_t *cpu ) {
    switch( cpu->op.pqxz.p ) {
        case 2:
            write8( cpu, READ_HL, cpu->a, 1 );
            SET_HL(READ_HL + 1);
            break;
        case 3:
            write8( cpu, READ_HL, cpu->a, 1 );
            SET_HL(READ_HL - 1);
            break;
        default:
            write8( cpu, read_rp( cpu, cpu->op.pqxz.p ), cpu->a, 1 );
    }
}

void op_add16( sm83_t *cpu ) {
    unsigned long oldhl = READ_HL;
    SET_HL( oldhl + read_rp( cpu, cpu->op.pqxz.p ) );
    SetFlags( cpu, cpu->f.flags.fz, 0, ( READ_HL & 0xfff ) < ( oldhl & 0xfff ), ( READ_HL & 0xffff ) < ( oldhl & 0xffff ) );
} 

void op_la16( sm83_t *cpu ) {
    switch( cpu->op.pqxz.p ) {
        case 2:
            cpu->a = read8( cpu, READ_HL, 1 );
            SET_HL(READ_HL + 1);
            break;
        case 3:
            cpu->a = read8( cpu, READ_HL, 1 );
            SET_HL(READ_HL - 1);
            break;
        default:
            cpu->a = read8( cpu, read_rp( cpu, cpu->op.pqxz.p ), 1 );
    }
}

void op_jp( sm83_t* cpu ) {
    unsigned long dest = read16( cpu, cpu->pc + 1, 1 );
    cpu->pc = dest - 1;
}

void op_jr( sm83_t* cpu ) {
    cpu->pc = cpu->pc + (char)read8( cpu, cpu->pc + 1, 1 ) + 1;
}

void op_jrx( sm83_t* cpu ) {
    if ( NzNc( cpu ) ) {
        cpu->pc = cpu->pc + (char)read8( cpu, cpu->pc + 1, 1 );
        cpu->timetarget += 4;
    }
    cpu->pc++;
}
void op_call( sm83_t* cpu ) {
    pushw( cpu, cpu->pc + 3 );
    cpu->pc = read16( cpu, cpu->pc + 1, 1 ) - 1;
}

void op_callx( sm83_t* cpu ) {
    if ( NzNc( cpu ) ) {
        pushw( cpu, cpu->pc + 3 );
        cpu->pc = read16( cpu, cpu->pc + 1, 1 ) - 1;
        cpu->timetarget += 12;
    } else {
        cpu->pc += 2;
    }
}

void op_jpx( sm83_t* cpu ) {
    if ( NzNc( cpu ) ) {
        cpu->pc = read16( cpu, cpu->pc + 1, 1 ) - 1;
        cpu->timetarget += 4;
    } else {
        cpu->pc += 2;
    }
}

void op_ret( sm83_t* cpu ) {
    cpu->pc = popw( cpu ) - 1;
}

void op_reti( sm83_t* cpu ) {
    cpu->pc = popw( cpu ) - 1;
    cpu->ifl = 1;
}

void op_retx( sm83_t* cpu ) {
    if ( NzNc( cpu ) ) {
        cpu->pc = popw( cpu ) - 1;
        cpu->timetarget += 12;
    }
}

void op_hlt( sm83_t *cpu ) {
    cpu->halted = 1;
}

void op_rst( sm83_t *cpu ) {
    pushw( cpu, cpu->pc + 1 );
    cpu->pc = (cpu->op.xyz.y << 3) - 1;
}

void op_pop( sm83_t *cpu ) {
    write_rp2( cpu, popw( cpu ), cpu->op.pqxz.p );
}

void op_psh( sm83_t *cpu ) {
    pushw( cpu, read_rp2( cpu, cpu->op.pqxz.p ) );
}

void op_edi( sm83_t *cpu ) {
    cpu->ifl = cpu->op.pqxz.q;
}

void op_stop( sm83_t *cpu ) {
    op_hlt( cpu );
}

void op_e0( sm83_t *cpu ) {
    write8( cpu, 0xFF00 + read8( cpu, ++cpu->pc, 1 ), cpu->a, 1 );
}

void op_e2( sm83_t *cpu ) {
    write8( cpu, 0xFF00 + cpu->c, cpu->a, 1 );
}

void op_ea( sm83_t *cpu ) {
    write8( cpu, read16( cpu, cpu->pc+1, 1 ), cpu->a, 1 );
    cpu->pc+=2;
}

void op_f0( sm83_t *cpu ) {
    cpu->a = read8( cpu, 0xFF00 + read8( cpu, ++cpu->pc, 1 ), 1 );
}

void op_f2( sm83_t *cpu ) {
    cpu->a = read8( cpu, 0xFF00 + cpu->c, 1 );
}

void op_fa( sm83_t *cpu ) {
    cpu->a = read8( cpu, read16( cpu, cpu->pc+1, 1 ), 1 );
    cpu->pc+=2;
}

void op_addsp( sm83_t *cpu ) {
    unsigned long oldsp = cpu->sp;
    char val = (char)read8( cpu, ++cpu->pc, 1 );
    cpu->sp = cpu->sp + val;
    SetFlags( cpu, 0, 0, ( oldsp & 0xf ) > ( cpu->sp & 0xf ), ( oldsp & 0xff ) > ( cpu->sp & 0xff ) );
}

void op_jphl( sm83_t *cpu ) {
    cpu->pc = READ_HL - 1;
}

void op_lhlsi( sm83_t *cpu ) {
    char val = (char)read8( cpu, ++cpu->pc, 1 );
    SET_HL( (short)cpu->sp + val );
    SetFlags( cpu, 0, 0, ( READ_HL & 0xf ) < ( cpu->sp & 0xf ), ( READ_HL & 0xff ) < ( cpu->sp & 0xff ) );
}

void op_l16sp( sm83_t *cpu ) {
    unsigned long addr = read16( cpu, cpu->pc+1, 1 );  
    write8( cpu, addr + 1, (cpu->sp & 0xff00) >> 8, 0 );
    write8( cpu, addr, cpu->sp & 0xff, 1 );
    cpu->pc += 2;
}

void op_lsphl( sm83_t *cpu ) {
    cpu->sp = READ_HL;
}

void op_rlc( sm83_t *cpu ) {
    unsigned char in = read_r( cpu, cpu->op.xyz.z ), newc = ( in & 0x80 ) >> 7, out = ( in << 1 ) | newc;
    write_r( cpu, out, cpu->op.xyz.z );
    SetFlags( cpu, out == 0, 0, 0, newc );
}

void op_rrc( sm83_t *cpu ) {
    unsigned char in = read_r( cpu, cpu->op.xyz.z ), newc = in & 0x1, out = ( in >> 1 ) | ( newc << 7 );
    SetFlags( cpu, out == 0, 0, 0, newc );
    write_r( cpu, out, cpu->op.xyz.z );
}

void op_rl( sm83_t *cpu ) {
    unsigned char in = read_r( cpu, cpu->op.xyz.z ), newc = ( in & 0x80 ) >> 7, out = ( in << 1 ) | cpu->f.flags.fc;
    write_r( cpu, out, cpu->op.xyz.z );
    SetFlags( cpu, out == 0, 0, 0, newc );
}

void op_rr( sm83_t *cpu ) {
    unsigned char in = read_r( cpu, cpu->op.xyz.z ), newc = in & 0x1, out = ( in >> 1 ) | ( cpu->f.flags.fc << 7 );
    write_r( cpu, out, cpu->op.xyz.z );  
    SetFlags( cpu, out == 0, 0, 0, newc );
}

void op_sla( sm83_t *cpu ) {
    unsigned char in = read_r( cpu, cpu->op.xyz.z ), newc = ( in & 0x80 ) >> 7, out = ( in << 1 ) & 0xFE;
    write_r( cpu, out, cpu->op.xyz.z );
    SetFlags( cpu, out == 0, 0, 0, newc );
}

void op_sra( sm83_t *cpu ) {
    unsigned char in = read_r( cpu, cpu->op.xyz.z ), newc = in & 0x1, out = ( in >> 1 ) | ( ( in & 0x80 ) );
    write_r( cpu, out, cpu->op.xyz.z );
    SetFlags( cpu, out == 0, 0, 0, newc );
}

void op_swap( sm83_t *cpu ) {
    unsigned char in = read_r( cpu, cpu->op.xyz.z ), upper = in >> 4, lower = in & 0x0F, out = ( lower << 4 ) | upper;
    write_r( cpu, out, cpu->op.xyz.z );
    SetFlags( cpu, out == 0, 0, 0, 0 );
}

void op_srl( sm83_t *cpu ) {
    unsigned char in = read_r( cpu, cpu->op.xyz.z ), newc = in & 0x1, out = ( in >> 1 ) & 0x7F;
    write_r( cpu, out, cpu->op.xyz.z );
    SetFlags( cpu, out == 0, 0, 0, newc );
}

void op_bit( sm83_t *cpu ) {
    SetFlags( cpu, ( read_r( cpu, cpu->op.xyz.z ) & ( 0x01 << cpu->op.xyz.y ) ) == 0 , 0, 1, cpu->f.flags.fc );
}

void op_res( sm83_t *cpu ) {
    write_r( cpu, read_r( cpu, cpu->op.xyz.z ) & ~( 0x01 << cpu->op.xyz.y ), cpu->op.xyz.z );
}

void op_set( sm83_t *cpu ) {
    write_r( cpu, read_r( cpu, cpu->op.xyz.z ) | ( 0x01 << cpu->op.xyz.y ), cpu->op.xyz.z );
}

void op_bad( sm83_t *cpu ) {
    printf( "bad instruction %02x at %04x\n", cpu->op.full, cpu->pc );
    exit( 1 );
}
