void op_nop( sm83_t *cpu );
void op_ld( sm83_t *cpu );
void op_ldd8( sm83_t *cpu );
void op_add( sm83_t *cpu );
void op_adc( sm83_t *cpu );
void op_sub( sm83_t *cpu );
void op_sbc( sm83_t *cpu );
void op_and( sm83_t *cpu );

void op_xor_b( sm83_t *cpu );
void op_xor_c( sm83_t *cpu );
void op_xor_d( sm83_t *cpu );
void op_xor_e( sm83_t *cpu );
void op_xor_h( sm83_t *cpu );
void op_xor_l( sm83_t *cpu );
void op_xor_r( sm83_t *cpu );
void op_xor_a( sm83_t *cpu );
void op_xor_i( sm83_t *cpu );

void op_or( sm83_t *cpu );
void op_cp( sm83_t *cpu );
void op_cpl( sm83_t *cpu );
void op_inc( sm83_t *cpu );
void op_dec( sm83_t *cpu );
void op_ccf( sm83_t *cpu );
void op_scf( sm83_t *cpu );
void op_daa( sm83_t *cpu );
void op_rlca( sm83_t *cpu );
void op_rla( sm83_t *cpu );
void op_rrca( sm83_t *cpu );
void op_rra( sm83_t *cpu );
void op_in16( sm83_t *cpu );
void op_de16( sm83_t *cpu );
void op_ld16( sm83_t *cpu );
void op_li16( sm83_t *cpu );
void op_add16( sm83_t *cpu );
void op_la16( sm83_t *cpu );
void op_jp( sm83_t* cpu );
void op_jr( sm83_t* cpu );
void op_jrx( sm83_t* cpu );
void op_call( sm83_t* cpu );
void op_callx( sm83_t* cpu );
void op_jpx( sm83_t* cpu );
void op_ret( sm83_t* cpu );
void op_reti( sm83_t* cpu );
void op_retx( sm83_t* cpu );
void op_hlt( sm83_t *cpu );
void op_rst( sm83_t *cpu );
void op_pop( sm83_t *cpu );
void op_psh( sm83_t *cpu );
void op_edi( sm83_t *cpu );
void op_stop( sm83_t *cpu );
void op_e0( sm83_t *cpu );
void op_e2( sm83_t *cpu );
void op_ea( sm83_t *cpu );
void op_f0( sm83_t *cpu );
void op_f2( sm83_t *cpu );
void op_fa( sm83_t *cpu );
void op_addsp( sm83_t *cpu );
void op_jphl( sm83_t *cpu );
void op_lhlsi( sm83_t *cpu );
void op_l16sp( sm83_t *cpu );
void op_lsphl( sm83_t *cpu );
void op_rlc( sm83_t *cpu );
void op_rrc( sm83_t *cpu );
void op_rl( sm83_t *cpu );
void op_rr( sm83_t *cpu );
void op_sla( sm83_t *cpu );
void op_sra( sm83_t *cpu );
void op_swap( sm83_t *cpu );
void op_srl( sm83_t *cpu );
void op_bit( sm83_t *cpu );
void op_res( sm83_t *cpu );
void op_set( sm83_t *cpu );
void op_bad( sm83_t *cpu );