typedef enum {
    MOS6502_REG_A = 0,
    MOS6502_REG_X = 1,
    MOS6502_REG_Y = 2,
} mosReg_t;

typedef struct mos6502_instruction_s {
    union {
        struct {
            uint8_t b7:1;
            uint8_t b6:1;
            uint8_t b5:1;
            uint8_t b4:1;
            uint8_t b3:1;
            uint8_t b2:1;
            uint8_t b1:1;
            uint8_t b0:1;
        };
        struct {
            uint8_t sb:3;
            uint8_t am:3;
            uint8_t g:2;
        };
        struct {
            uint8_t w:2;
            uint8_t e:1;
            uint8_t b:6;
        };
        uint8_t full;
    };
} mos6502_instruction_t;

typedef struct mos6502_s { 
    // registers
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
    // internal state tracking
    mos6502_instruction_t op;
    uint8_t target_reg;

    // external interface
    busDevice_t* bus;
    void (*Reset)( struct mos6502_s *self );
    void (*Step)( struct mos6502_s *self );
    void (*Interrupt)( struct mos6502_s *self );
    void (*Nmi)( struct mos6502_s *self );
} mos6502_t;

mos6502_t *Mos6502( busDevice_t *bus );