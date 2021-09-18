#include <stdio.h>
#include <stdlib.h>

#include "device.h"

#include "sm83.h"
#include "sm83_ops.h"

/* SUPPORT */

#define write8( cpu, addr, val, final ) cpu->bus->Write8( cpu->bus, addr, val, final )
#define read8( cpu, addr, final ) cpu->bus->Read8( cpu->bus, addr, final )
#define read16( cpu, addr, final ) ( ( cpu->bus->Read8( cpu->bus, addr + 1, 0 ) << 8 ) | cpu->bus->Read8( cpu->bus, addr, final ) )

#define READ_HL ( ( cpu->h << 8 ) + cpu->l )
#define SET_HL(val) cpu->h = ( (val) >> 8); cpu->l = ((val) & 0xff)

#define CPU_BITS_P(cpu) ( ( cpu->op >> 4 ) & 0x03 )
#define CPU_BITS_Q(cpu) ( ( cpu->op >> 3 ) & 0x01 )

#define CPU_BITS_X(cpu) ( ( cpu->op >> 6 ) & 0x03 )
#define CPU_BITS_Y(cpu) ( ( cpu->op >> 3 ) & 0x07 )
#define CPU_BITS_Z(cpu) ( cpu->op & 0x07 )

#define CPU_FLAG_Z(cpu) ( ( cpu->f >> 7 ) & 0x01 )
#define CPU_FLAG_N(cpu) ( ( cpu->f >> 6 ) & 0x01 )
#define CPU_FLAG_H(cpu) ( ( cpu->f >> 5 ) & 0x01 )
#define CPU_FLAG_C(cpu) ( ( cpu->f >> 4 ) & 0x01 )

void SetFlags( sm83_t *cpu, unsigned char z, unsigned char n, unsigned char h, unsigned char c ) {
    cpu->f = ( ( z & 0x01 ) << 7 ) | ( ( n & 0x01 ) << 6 ) | ( ( h & 0x01 ) << 5 ) | ( ( c & 0x01 ) << 4 );
}

/* decode logic for condition on zero/carry */
int NzNc( sm83_t *cpu ) {
    int cond = ( ( ( CPU_BITS_P(cpu) & 1 ) == 0 ) ? CPU_FLAG_Z(cpu) : CPU_FLAG_C(cpu) );
    return ( CPU_BITS_Q(cpu) == 0 ) ? !cond : cond;
}

/* OPCODES */

void op_nop( sm83_t *cpu ) {}

void op_ld( sm83_t *cpu ) {
    write_r( cpu, read_r( cpu, CPU_BITS_Z(cpu) ), CPU_BITS_Y(cpu));
}

void op_ldd8( sm83_t *cpu ) {
    write_r( cpu, read8( cpu, cpu->pc++ + 1, 0 ), CPU_BITS_Y(cpu));
}

void op_add( sm83_t *cpu ) {
    unsigned char other = ( CPU_BITS_X(cpu) == 3 ) ? read8( cpu, ++cpu->pc, 1 ) : read_r( cpu, CPU_BITS_Z(cpu) );
    unsigned char orig = cpu->a, res = cpu->a = cpu->a + other;
    SetFlags( cpu, res == 0, 0, ( res & 0xf ) < ( orig & 0xf ), res < orig );
}

void op_adc( sm83_t *cpu ) {
    unsigned char other = ( CPU_BITS_X(cpu) == 3 ) ? read8( cpu, ++cpu->pc, 1 ) : read_r( cpu, CPU_BITS_Z(cpu) );
    unsigned char orig = cpu->a, res = cpu->a = cpu->a + other + CPU_FLAG_C(cpu);
    SetFlags( cpu, res == 0, 0, ( other & 0xf ) + ( orig & 0xf ) + CPU_FLAG_C(cpu) > 0xf, ((unsigned long)orig + (unsigned long)other + (unsigned long)CPU_FLAG_C(cpu)) > 0xFF);
}

void op_sub( sm83_t *cpu ) {
    unsigned char other = ( CPU_BITS_X(cpu) == 3 ) ? read8( cpu, ++cpu->pc, 1 ) : read_r( cpu, CPU_BITS_Z(cpu) );
    unsigned char orig = cpu->a, res = cpu->a = cpu->a - other;
    SetFlags( cpu, res == 0, 1, ( res & 0xf ) > ( orig & 0xf ), res > orig );
}

void op_sbc( sm83_t *cpu ) {
    unsigned char other = ( CPU_BITS_X(cpu) == 3 ) ? read8( cpu, ++cpu->pc, 1 ) : read_r( cpu, CPU_BITS_Z(cpu) );
    unsigned char orig = cpu->a, res = cpu->a = ( cpu->a - other ) - CPU_FLAG_C(cpu);
    SetFlags( cpu, res == 0, 1, (( other & 0xf ) + CPU_FLAG_C(cpu) ) > ( orig & 0xf ), ((orig - other) - CPU_FLAG_C(cpu)) < 0 );
}

void op_and_b( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a & cpu->b ) ) == 0 ), 0, 1, 0 ); }
void op_and_c( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a & cpu->c ) ) == 0 ), 0, 1, 0 ); }
void op_and_d( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a & cpu->d ) ) == 0 ), 0, 1, 0 ); }
void op_and_e( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a & cpu->e ) ) == 0 ), 0, 1, 0 ); }
void op_and_h( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a & cpu->h ) ) == 0 ), 0, 1, 0 ); }
void op_and_l( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a & cpu->l ) ) == 0 ), 0, 1, 0 ); }
void op_and_r( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a & read8( cpu, READ_HL, 1 ) ) ) == 0 ), 0, 1, 0 ); }
void op_and_a( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a & cpu->a ) ) == 0 ), 0, 1, 0 ); }
void op_and_i( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a & read8( cpu, ++cpu->pc, 1 ) ) ) == 0 ), 0, 1, 0 ); }

void op_xor_b( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a ^ cpu->b ) ) == 0 ), 0, 0, 0 ); }
void op_xor_c( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a ^ cpu->c ) ) == 0 ), 0, 0, 0 ); }
void op_xor_d( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a ^ cpu->d ) ) == 0 ), 0, 0, 0 ); }
void op_xor_e( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a ^ cpu->e ) ) == 0 ), 0, 0, 0 ); }
void op_xor_h( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a ^ cpu->h ) ) == 0 ), 0, 0, 0 ); }
void op_xor_l( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a ^ cpu->l ) ) == 0 ), 0, 0, 0 ); }
void op_xor_r( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a ^ read8( cpu, READ_HL, 1 ) ) ) == 0 ), 0, 0, 0 ); }
void op_xor_a( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a ^ cpu->a ) ) == 0 ), 0, 0, 0 ); }
void op_xor_i( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a ^ read8( cpu, ++cpu->pc, 1 ) ) ) == 0 ), 0, 0, 0 ); }

void op_or_b( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a | cpu->b ) ) == 0 ), 0, 0, 0 ); }
void op_or_c( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a | cpu->c ) ) == 0 ), 0, 0, 0 ); }
void op_or_d( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a | cpu->d ) ) == 0 ), 0, 0, 0 ); }
void op_or_e( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a | cpu->e ) ) == 0 ), 0, 0, 0 ); }
void op_or_h( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a | cpu->h ) ) == 0 ), 0, 0, 0 ); }
void op_or_l( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a | cpu->l ) ) == 0 ), 0, 0, 0 ); }
void op_or_r( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a | read8( cpu, READ_HL, 1 ) ) ) == 0 ), 0, 0, 0 ); }
void op_or_a( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a | cpu->a ) ) == 0 ), 0, 0, 0 ); }
void op_or_i( sm83_t *cpu ) { SetFlags( cpu, ( ( cpu->a = ( cpu->a | read8( cpu, ++cpu->pc, 1 ) ) ) == 0 ), 0, 0, 0 ); }

void op_cp( sm83_t *cpu ) {
    unsigned char other = ( CPU_BITS_X(cpu) == 3 ) ? read8( cpu, ++cpu->pc, 1 ) : read_r( cpu, CPU_BITS_Z(cpu) );
    unsigned char res = cpu->a - other;
    SetFlags( cpu, ( res == 0 ), 1, ( res & 0x0f ) > ( cpu->a & 0x0f ), ( res ) > ( cpu->a ) );
}

void op_cpl( sm83_t *cpu ) {
    cpu->a = ~cpu->a;
    SetFlags( cpu, CPU_FLAG_Z(cpu), 1, 1, CPU_FLAG_C(cpu) );
}

void op_inc( sm83_t *cpu ) {
    unsigned char in = read_r( cpu, CPU_BITS_Y(cpu)), res = in + 1;
    SetFlags( cpu, res == 0, 0, ( res & 0xf ) < ( in & 0xf), CPU_FLAG_C(cpu) );
    write_r( cpu, res, CPU_BITS_Y(cpu));
}

void op_dec( sm83_t *cpu ) {
    unsigned char in = read_r( cpu, CPU_BITS_Y(cpu)), res = in - 1;
    SetFlags( cpu, res == 0, 1, ( res & 0x0f ) > ( in & 0x0f), CPU_FLAG_C(cpu) );
    write_r( cpu, res, CPU_BITS_Y(cpu));
}

void op_ccf( sm83_t *cpu ) {
    SetFlags( cpu, CPU_FLAG_Z(cpu), 0, 0, CPU_FLAG_C(cpu) ^ 1 );
}

void op_scf( sm83_t *cpu ) {
    SetFlags( cpu, CPU_FLAG_Z(cpu), 0, 0, 1 );
}

void op_daa( sm83_t *cpu ) {
    int carry = CPU_FLAG_C(cpu);
    if ( CPU_FLAG_N(cpu) ) {
        cpu->a = ( cpu->a - ( CPU_FLAG_C(cpu) ? 0x60 : 0 ) ) - ( CPU_FLAG_H(cpu) ? 0x6 : 0 );
    } else {
        if ( CPU_FLAG_C(cpu) || ( cpu->a > 0x99 ) ) {
            cpu->a += 0x60;
            carry = 1;
        }
        if ( CPU_FLAG_H(cpu) || ( ( cpu->a & 0xf ) > 0x9 ) ) {
            cpu->a += 0x6;
        }
    }
    SetFlags( cpu, cpu->a == 0, CPU_FLAG_N(cpu), 0, carry );
}

void op_rlca( sm83_t *cpu ) {
    SetFlags( cpu, 0, 0, 0, ( cpu->a & 0x80 ) >> 7 );
    cpu->a = ( cpu->a << 1 ) + CPU_FLAG_C(cpu);
}

void op_rla( sm83_t *cpu ) {
    unsigned char newc = ( cpu->a & 0x80 ) >> 7;
    cpu->a = ( cpu->a << 1 ) + CPU_FLAG_C(cpu);
    SetFlags( cpu, 0, 0, 0, newc );
}

void op_rrca( sm83_t *cpu ) {
    SetFlags( cpu, 0, 0, 0, ( cpu->a & 0x01 ) );
    cpu->a = ( cpu->a >> 1 ) | ( ( cpu->a & 0x01 ) << 7 );
}

void op_rra( sm83_t *cpu ) {
    unsigned char newc = ( cpu->a & 0x01 );
    cpu->a = ( cpu->a >> 1 ) + ( CPU_FLAG_C(cpu) << 7 );
    SetFlags( cpu, 0, 0, 0, newc );
}

void op_in16( sm83_t *cpu ) {
    write_rp( cpu, read_rp( cpu, CPU_BITS_P(cpu) ) + 1, CPU_BITS_P(cpu) );
}

void op_de16( sm83_t *cpu ) {
    write_rp( cpu, read_rp( cpu, CPU_BITS_P(cpu) ) - 1, CPU_BITS_P(cpu) );
}

void op_ld16( sm83_t *cpu ) {
    write_rp( cpu, read16( cpu, ( cpu->pc+1 ), 1 ), CPU_BITS_P(cpu) );
    cpu->pc+=2;
}

void op_li16( sm83_t *cpu ) {
    switch( CPU_BITS_P(cpu) ) {
        case 2:
            write8( cpu, READ_HL, cpu->a, 1 );
            SET_HL(READ_HL + 1);
            break;
        case 3:
            write8( cpu, READ_HL, cpu->a, 1 );
            SET_HL(READ_HL - 1);
            break;
        default:
            write8( cpu, read_rp( cpu, CPU_BITS_P(cpu) ), cpu->a, 1 );
    }
}

void op_add16( sm83_t *cpu ) {
    unsigned long oldhl = READ_HL;
    SET_HL( oldhl + read_rp( cpu, CPU_BITS_P(cpu) ) );
    SetFlags( cpu, CPU_FLAG_Z(cpu), 0, ( READ_HL & 0xfff ) < ( oldhl & 0xfff ), ( READ_HL & 0xffff ) < ( oldhl & 0xffff ) );
} 

void op_la16( sm83_t *cpu ) {
    switch( CPU_BITS_P(cpu) ) {
        case 2:
            cpu->a = read8( cpu, READ_HL, 1 );
            SET_HL(READ_HL + 1);
            break;
        case 3:
            cpu->a = read8( cpu, READ_HL, 1 );
            SET_HL(READ_HL - 1);
            break;
        default:
            cpu->a = read8( cpu, read_rp( cpu, CPU_BITS_P(cpu) ), 1 );
    }
}

void op_jp( sm83_t* cpu ) {
    unsigned long dest = read16( cpu, cpu->pc + 1, 1 );
    cpu->pc = dest - 1;
}

void op_jr( sm83_t* cpu ) {
    cpu->pc = ( cpu->pc + (signed char)read8( cpu, cpu->pc + 1, 1 ) ) + 1;
}

void op_jrx( sm83_t* cpu ) {
    if ( NzNc( cpu ) ) {
        cpu->pc = cpu->pc + (signed char)read8( cpu, cpu->pc + 1, 1 );
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
    cpu->pc = (CPU_BITS_Y(cpu)<< 3) - 1;
}

void op_pop( sm83_t *cpu ) {
    write_rp2( cpu, popw( cpu ), CPU_BITS_P(cpu) );
}

void op_psh( sm83_t *cpu ) {
    pushw( cpu, read_rp2( cpu, CPU_BITS_P(cpu) ) );
}

void op_edi( sm83_t *cpu ) {
    cpu->ifl_next = CPU_BITS_Q(cpu);
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
    char val = (signed char)read8( cpu, ++cpu->pc, 1 );
    cpu->sp = cpu->sp + val;
    SetFlags( cpu, 0, 0, ( oldsp & 0xf ) > ( cpu->sp & 0xf ), ( oldsp & 0xff ) > ( cpu->sp & 0xff ) );
}

void op_jphl( sm83_t *cpu ) {
    cpu->pc = READ_HL - 1;
}

void op_lhlsi( sm83_t *cpu ) {
    char val = (signed char)read8( cpu, ++cpu->pc, 1 );
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
    unsigned char in = read_r( cpu, CPU_BITS_Z(cpu) ), newc = ( in & 0x80 ) >> 7, out = ( in << 1 ) | newc;
    write_r( cpu, out, CPU_BITS_Z(cpu) );
    SetFlags( cpu, out == 0, 0, 0, newc );
}

void op_rrc( sm83_t *cpu ) {
    unsigned char in = read_r( cpu, CPU_BITS_Z(cpu) ), newc = in & 0x1, out = ( in >> 1 ) | ( newc << 7 );
    SetFlags( cpu, out == 0, 0, 0, newc );
    write_r( cpu, out, CPU_BITS_Z(cpu) );
}

void op_rl( sm83_t *cpu ) {
    unsigned char in = read_r( cpu, CPU_BITS_Z(cpu) ), newc = ( in & 0x80 ) >> 7, out = ( in << 1 ) | CPU_FLAG_C(cpu);
    write_r( cpu, out, CPU_BITS_Z(cpu) );
    SetFlags( cpu, out == 0, 0, 0, newc );
}

void op_rr( sm83_t *cpu ) {
    unsigned char in = read_r( cpu, CPU_BITS_Z(cpu) ), newc = in & 0x1, out = ( in >> 1 ) | ( CPU_FLAG_C(cpu) << 7 );
    write_r( cpu, out, CPU_BITS_Z(cpu) );  
    SetFlags( cpu, out == 0, 0, 0, newc );
}

void op_sla( sm83_t *cpu ) {
    unsigned char in = read_r( cpu, CPU_BITS_Z(cpu) ), newc = ( in & 0x80 ) >> 7, out = ( in << 1 ) & 0xFE;
    write_r( cpu, out, CPU_BITS_Z(cpu) );
    SetFlags( cpu, out == 0, 0, 0, newc );
}

void op_sra( sm83_t *cpu ) {
    unsigned char in = read_r( cpu, CPU_BITS_Z(cpu) ), newc = in & 0x1, out = ( in >> 1 ) | ( ( in & 0x80 ) );
    write_r( cpu, out, CPU_BITS_Z(cpu) );
    SetFlags( cpu, out == 0, 0, 0, newc );
}

void op_swap( sm83_t *cpu ) {
    unsigned char in = read_r( cpu, CPU_BITS_Z(cpu) ), upper = in >> 4, lower = in & 0x0F, out = ( lower << 4 ) | upper;
    write_r( cpu, out, CPU_BITS_Z(cpu) );
    SetFlags( cpu, out == 0, 0, 0, 0 );
}

void op_srl( sm83_t *cpu ) {
    unsigned char in = read_r( cpu, CPU_BITS_Z(cpu) ), newc = in & 0x1, out = ( in >> 1 ) & 0x7F;
    write_r( cpu, out, CPU_BITS_Z(cpu) );
    SetFlags( cpu, out == 0, 0, 0, newc );
}

void op_bit( sm83_t *cpu ) {
    SetFlags( cpu, ( read_r( cpu, CPU_BITS_Z(cpu) ) & ( 0x01 << CPU_BITS_Y(cpu)) ) == 0 , 0, 1, CPU_FLAG_C(cpu) );
}

void op_res( sm83_t *cpu ) {
    write_r( cpu, read_r( cpu, CPU_BITS_Z(cpu) ) & ~( 0x01 << CPU_BITS_Y(cpu)), CPU_BITS_Z(cpu) );
}

void op_set( sm83_t *cpu ) {
    write_r( cpu, read_r( cpu, CPU_BITS_Z(cpu) ) | ( 0x01 << CPU_BITS_Y(cpu)), CPU_BITS_Z(cpu) );
}

void op_bad( sm83_t *cpu ) {
    printf( "bad instruction %02x at %04x\n", cpu->op, cpu->pc );
    exit( 1 );
}
