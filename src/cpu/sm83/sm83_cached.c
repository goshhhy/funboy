#include <stdio.h>
#include <stdlib.h>

#include "device.h"

#include "sm83.h"
#include "sm83_ops.h"

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

void sm83_fused_op_f0f0( sm83_t * cpu ) {
    op_f0( cpu );
    cpu->pc++;
    op_f0( cpu );
}

void sm83_fused_op_f0a7( sm83_t * cpu ) {
    op_f0( cpu );
    cpu->pc++;
    op_and_a( cpu );
}

sm83_fused_op_t sm83_fused_ops[] = {
    { 0xf0, 0xf0, sm83_fused_op_f0f0 },
    { 0xf0, 0xa7, sm83_fused_op_f0a7 },
    { 0, 0, NULL },
};