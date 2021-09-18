#include <stdio.h>
#include <stdlib.h>

#include "device.h"

#include "sm83.h"
#include "sm83_ops.h"

#define write8( cpu, addr, val, final ) cpu->bus->Write8( cpu->bus, addr, val, final )
#define read8( cpu, addr, final ) cpu->bus->Read8( cpu->bus, addr, final )
#define read16( cpu, addr, final ) ( ( cpu->bus->Read8( cpu->bus, addr + 1, 0 ) << 8 ) | cpu->bus->Read8( cpu->bus, addr, final ) )

extern int printSteps;

/* support functions */

unsigned char read_r( sm83_t *cpu, unsigned char r ) {
    switch( r ) {
    	case LD_MODE_B: 
    		return cpu->b;
    	case LD_MODE_C: 
    		return cpu->c;
    	case LD_MODE_D:
    		return cpu->d;
    	case LD_MODE_E:
    		return cpu->e;
    	case LD_MODE_H:
    		return cpu->h;
    	case LD_MODE_L:
    		return cpu->l;
    	case LD_MODE_HL:
    		return read8( cpu, ( ( cpu->h << 8 ) + cpu->l ), 1 );
    	case LD_MODE_A:
    		return cpu->a;
    	default:
    		printf("warning: sm83: invalid ld mode\n");
    		return 0;
    }
}

void write_r( sm83_t *cpu, unsigned char val, unsigned char r ) {
    switch( r ) {
    	case LD_MODE_B:
    		cpu->b = val;
    		break;
    	case LD_MODE_C: 
    		cpu->c = val;
    		break;
    	case LD_MODE_D:
    		cpu->d = val;
    		break;
    	case LD_MODE_E:
    		cpu->e = val;
    		break;
    	case LD_MODE_H:
    		cpu->h = val;
    		break;
    	case LD_MODE_L:
    		cpu->l = val;
    		break;
    	case LD_MODE_HL:
    		write8( cpu, ( ( cpu->h << 8 ) + cpu->l ), val, 1 );
    		break;
    	case LD_MODE_A:
    		cpu->a = val;
    		break;
    	default:
    		printf("warning: sm83: invalid ld mode\n");
    }
}

unsigned short read_rp( sm83_t *cpu, unsigned char r ) {
    switch( r ) {
        case 0:
            return (cpu->b << 8) + cpu->c;
        case 1:
            return (cpu->d << 8) + cpu->e;
        case 2:
            return (cpu->h << 8) + cpu->l;
        case 3:
            return cpu->sp;
        default:
            printf("warning: sm83: invalid double register reference\n");
            return 0;
    }
}

void write_rp( sm83_t *cpu, unsigned short val, unsigned char r ) {
    switch( r ) {
        case 0:
            cpu->b = (val >> 8);
            cpu->c = (val & 0xff);
            break;
        case 1:
            cpu->d = (val >> 8);
            cpu->e = (val & 0xff);
            break;
        case 2:
            cpu->h = (val >> 8);
            cpu->l = (val & 0xff);
            break;
        case 3:
            cpu->sp = val;
            break;
        default:
            printf("warning: sm83: invalid double register reference\n");
    }
}

unsigned short read_rp2( sm83_t *cpu, unsigned char r ) {
    switch( r ) {
        case 0:
            return (cpu->b << 8) + cpu->c;
        case 1:
            return (cpu->d << 8) + cpu->e;
        case 2:
            return (cpu->h << 8) + cpu->l;
        case 3:
            return (cpu->a << 8) + cpu->f;
        default:
            printf("warning: sm83: invalid double register reference\n");
            return 0;
    }
}

void write_rp2( sm83_t *cpu, unsigned short val, unsigned char r ) {
    switch( r ) {
        case 0:
            cpu->b = (val >> 8);
            cpu->c = (val & 0xff);
            break;
        case 1:
            cpu->d = (val >> 8);
            cpu->e = (val & 0xff);
            break;
        case 2:
            cpu->h = (val >> 8);
            cpu->l = (val & 0xff);
            break;
        case 3:
            cpu->a = (val >> 8);
            cpu->f = (val & 0xf0);
            break;
        default:
            printf("warning: sm83: invalid double register reference\n");
    }
}

void pushw( sm83_t* cpu, unsigned short word ) {
    cpu->sp -= 2;
    write8( cpu, cpu->sp + 1, ( word & 0xff00 ) >> 8, 0 );
    write8( cpu, cpu->sp, ( word & 0xff ), 1 );
}

unsigned short popw( sm83_t* cpu ) {
    cpu->sp += 2;
    return ( read8( cpu, cpu->sp - 1, 0) << 8 ) | read8( cpu, cpu->sp - 2, 1 );
}

/* op tables */

void (*cbops[32])( sm83_t* cpu ) = {
    op_rlc, op_rrc, op_rl, op_rr, op_sla, op_sra, op_swap, op_srl,
    op_bit, op_bit, op_bit, op_bit, op_bit, op_bit, op_bit, op_bit,
    op_res, op_res, op_res, op_res, op_res, op_res, op_res, op_res,
    op_set, op_set, op_set, op_set, op_set, op_set, op_set, op_set,
};

static void op_cb( sm83_t *cpu ) {
    cpu->pc++;
    cpu->op = read8( cpu, cpu->pc, 1 );
    cbops[( cpu->op & 0xF8 ) >> 3]( cpu );
}

void (*ops[256])( sm83_t* cpu ) = {
    op_nop,   op_ld16,  op_li16,  op_in16,   op_inc_b, op_dec_b, op_ldd8,  op_rlca,     op_l16sp, op_add16, op_la16,  op_de16,    op_inc_c, op_dec_c, op_ldd8,  op_rrca, 
    op_stop,  op_ld16,  op_li16,  op_in16,   op_inc_d, op_dec_d, op_ldd8,  op_rla,      op_jr,    op_add16, op_la16,  op_de16,    op_inc_e, op_dec_e, op_ldd8,  op_rra, 
    op_jrx,   op_ld16,  op_li16,  op_in16,   op_inc_h, op_dec_h, op_ldd8,  op_daa,      op_jrx,   op_add16, op_la16,  op_de16,    op_inc_l, op_dec_l, op_ldd8,  op_cpl, 
    op_jrx,   op_ld16,  op_li16,  op_in16,   op_inc_hl,op_dec_hl,op_ldd8,  op_scf,      op_jrx,   op_add16, op_la16,  op_de16,    op_inc_a, op_dec_a, op_ldd8,  op_ccf, 

    op_nop,   op_ld_b_c,op_ld_b_d,op_ld_b_e, op_ld_b_h,op_ld_b_l,op_ld_b_r,op_ld_b_a,   op_ld_c_b,op_nop,   op_ld_c_d,op_ld_c_e,  op_ld_c_h,op_ld_c_l,op_ld_c_r,op_ld_c_a,  
    op_ld_d_b,op_ld_d_c,op_nop,   op_ld_d_e, op_ld_d_h,op_ld_d_l,op_ld_d_r,op_ld_d_a,   op_ld_e_b,op_ld_e_c,op_ld_e_d,op_nop,     op_ld_e_h,op_ld_e_l,op_ld_e_r,op_ld_e_a,  
    op_ld_h_b,op_ld_h_c,op_ld_h_d,op_ld_h_e, op_nop,   op_ld_h_l,op_ld_h_r,op_ld_h_a,   op_ld_l_b,op_ld_l_c,op_ld_l_d,op_ld_l_e,  op_ld_l_h,op_nop,   op_ld_l_r,op_ld_l_a,  
    op_ld_r_b,op_ld_r_c,op_ld_r_d,op_ld_r_e, op_ld_r_h,op_ld_r_l,op_hlt,   op_ld_r_a,   op_ld_a_b,op_ld_a_c,op_ld_a_d,op_ld_a_e,  op_ld_a_h,op_ld_a_l,op_ld_a_r,op_nop,

    op_add,   op_add,   op_add,   op_add,    op_add,   op_add,   op_add,   op_add,      op_adc,   op_adc,   op_adc,   op_adc,     op_adc,   op_adc,   op_adc,   op_adc, 
    op_sub,   op_sub,   op_sub,   op_sub,    op_sub,   op_sub,   op_sub,   op_sub,      op_sbc,   op_sbc,   op_sbc,   op_sbc,     op_sbc,   op_sbc,   op_sbc,   op_sbc, 
    op_and_b, op_and_c, op_and_d, op_and_e,  op_and_h, op_and_l, op_and_r, op_and_a,    op_xor_b, op_xor_c, op_xor_d, op_xor_e,   op_xor_h, op_xor_l, op_xor_r, op_xor_a, 
    op_or_b,  op_or_c,  op_or_d,  op_or_e,   op_or_h,  op_or_l,  op_or_r,  op_or_a,     op_cp,    op_cp,    op_cp,    op_cp,      op_cp,    op_cp,    op_cp,    op_cp,  

    op_retx,  op_pop,   op_jpx,   op_jp,     op_callx, op_psh,   op_add,   op_rst,      op_retx,  op_ret,   op_jpx,   op_cb,      op_callx, op_call,  op_adc,   op_rst, 
    op_retx,  op_pop,   op_jpx,   op_bad,    op_callx, op_psh,   op_sub,   op_rst,      op_retx,  op_reti,  op_jpx,   op_bad,     op_callx, op_bad,   op_sbc,   op_rst, 
    op_e0,    op_pop,   op_e2,    op_bad,    op_bad,   op_psh,   op_and_i, op_rst,      op_addsp, op_jphl,  op_ea,    op_bad,     op_bad,   op_bad,   op_xor_i, op_rst, 
    op_f0,    op_pop,   op_f2,    op_edi,    op_bad,   op_psh,   op_or_i,  op_rst,      op_lhlsi, op_lsphl, op_fa,    op_edi,     op_bad,   op_bad,   op_cp,    op_rst, 
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
    unsigned char active;

    if ( cpu->fetched == 0 ) {
        cpu->op = cpu->bus->Read8( cpu->bus, cpu->pc, 0 );
        cpu->timetarget += timings[cpu->op];
        cpu->fetched = 1;
    }

    cpu->timetarget--;
    if ( cpu->timetarget > 0 )
        return;

    if ( ! cpu->halted ) {
    	/*
        printf("[%04x] %02x b:%02x c:%02x d:%02x e:%02x h:%02x l:%02x a:%02x f:%02x\n", 
    				cpu->pc, cpu->op,
    				cpu->b, cpu->c, cpu->d, cpu->e, cpu->h, cpu->l, cpu->a, cpu->f );
        */
        ops[cpu->op]( cpu );
        cpu->pc++;
    }

    /* run interrupts if applicable */
    if ( ( cpu->ifl == 1 ) || ( cpu->halted ) ) {
        active = ( read8( cpu, 0xffff, 0 ) & read8( cpu, 0xff0f, 0 ) ) & 0x1F; 
        if ( active ) {
            /*printf("running interrupt\n");*/
            pushw( cpu, cpu->pc );
            cpu->halted = 0;
            cpu->ifl = 0;
            if ( active & 0x1 ) {
                write8( cpu, 0xff0f, active & 0x1E, 1 );
                cpu->pc = 0x40;
            } else if ( active & 0x2 ) {
                write8( cpu, 0xff0f, active & 0x1D, 1 );
                cpu->pc = 0x48;
            } else if ( active & 0x4 ) {
                write8( cpu, 0xff0f, active & 0x1B, 1 );
                cpu->pc = 0x50;
            } else if ( active & 0x8 ) {
                write8( cpu, 0xff0f, active & 0x17, 1 );
                cpu->pc = 0x58;
            } else if ( active & 0x10 ) {
                write8( cpu, 0xff0f, active & 0x0F, 1 );
                cpu->pc = 0x60;
            }
            cpu->timetarget += 0;
        }
    }
    cpu->ifl = cpu->ifl_next;
    cpu->fetched = 0;
}

static unsigned long StepMultiple( struct sm83_s *cpu, unsigned long tcycles, unsigned long * i, int * stopFlag ) {
    unsigned char active;

    for (*i = 0; ( *i < tcycles ) && (!(*stopFlag)); ) {
        if ( cpu->fetched == 0 ) {
            cpu->op = cpu->bus->Read8( cpu->bus, cpu->pc, 0 );
            cpu->timetarget += timings[cpu->op];
            cpu->fetched = 1;
        }

        if ( cpu->timetarget > ( tcycles - *i ) ) {
            cpu->timetarget -= tcycles - *i;
            *i = tcycles;
            break;
        }

        *i += cpu->timetarget;
        cpu->timetarget = 0;

        if ( ! cpu->halted ) {
            ops[cpu->op]( cpu );
            cpu->pc++;
        }

        /* run interrupts if applicable */
        if ( ( cpu->ifl == 1 ) || ( cpu->halted ) ) {
            active = ( read8( cpu, 0xffff, 0 ) & read8( cpu, 0xff0f, 0 ) ) & 0x1F; 
            if ( active ) {
                /*printf("running interrupt\n");*/
                pushw( cpu, cpu->pc );
                cpu->halted = 0;
                cpu->ifl = 0;
                if ( active & 0x1 ) {
                    write8( cpu, 0xff0f, active & 0x1E, 1 );
                    cpu->pc = 0x40;
                } else if ( active & 0x2 ) {
                    write8( cpu, 0xff0f, active & 0x1D, 1 );
                    cpu->pc = 0x48;
                } else if ( active & 0x4 ) {
                    write8( cpu, 0xff0f, active & 0x1B, 1 );
                    cpu->pc = 0x50;
                } else if ( active & 0x8 ) {
                    write8( cpu, 0xff0f, active & 0x17, 1 );
                    cpu->pc = 0x58;
                } else if ( active & 0x10 ) {
                    write8( cpu, 0xff0f, active & 0x0F, 1 );
                    cpu->pc = 0x60;
                }
                cpu->timetarget += 0;
            }
        }
        cpu->ifl = cpu->ifl_next;
        cpu->fetched = 0;
    }

    return *i;
}

static void Interrupt( sm83_t *cpu, unsigned char inum ) {
        /*fprintf( stderr, "interrupt %hhi set with interrupts %i, ie %hhi\n", inum, cpu->ifl, read8( cpu, 0xffff, 0 ) );*/
        write8( cpu, 0xff0f, read8( cpu, 0xff0f, 0 ) | (0x1 << inum), 0 );
        cpu->halted = 0;
}

static void Reset( sm83_t *cpu ) {
    cpu->a = 0x01; cpu->f = 0xb0;
    cpu->b = 0x00; cpu->c = 0x13;
    cpu->d = 0x00; cpu->e = 0xd8;
    cpu->h = 0x01; cpu->l = 0x4d;
    cpu->sp = 0xfffe;
    cpu->pc = 0x0100;
    cpu->halted = 0;
    cpu->fetched = 0;
    cpu->timetarget = 1;
}

sm83_t *Sm83( busDevice_t *bus ) {
	sm83_t *cpu = calloc( 1, sizeof( sm83_t ) );
	cpu->bus = bus;
	cpu->Reset = Reset;
	cpu->Step = Step;
    cpu->StepMultiple = StepMultiple;
    cpu->Interrupt = Interrupt;
	Reset( cpu );
    printf( "sm83 cpu created\n" );
	return cpu;
}
