typedef struct mos6502_s { 
    uint8_t a, x, y, sp;
    union {
		struct {
			uint8_t sc:1;
			uint8_t sz:1;
			uint8_t si:1;
			uint8_t sd:1;
			uint8_t sb:1;
			uint8_t s_:1;
			uint8_t sv:1;
			uint8_t sn:1;
		};
		uint8_t status;
	};
	uint16_t pc;
    busDevice_t* bus;
    void (*Reset)( struct mos6502_s *self );
    void (*Step)( struct mos6502_s *self );
    void (*Interrupt)( struct mos6502_s *self );
    void (*Nmi)( struct mos6502_s *self );
} mos6502_t;

mos6502_t *Mos6502( busDevice_t *bus );