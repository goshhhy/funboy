
#define lr_dreg(x, y) union{struct{uint8_t y; uint8_t x;};uint16_t x##y;};

typedef struct lr35902_instruction_s {
    union {
        struct {
            uint8_t b0:1;
            uint8_t b1:1;
            uint8_t b2:1;
            uint8_t b3:1;
            uint8_t b4:1;
            uint8_t b5:1;
            uint8_t b6:1;
            uint8_t b7:1;
        };
        struct {
            uint8_t z:3;
			uint8_t y:3;
            uint8_t x:2;
        };
		struct {
			uint8_t z2:3;
			uint8_t q:1;
			uint8_t p:2;
			uint8_t x2:2;
		};
        uint8_t full;
    };
} lr35902_instruction_t;

typedef struct lr35902_s{ 
	union{
		struct{
			union{
				struct{
					uint8_t fl:4;
					uint8_t fc:1;
					uint8_t fh:1;
					uint8_t fn:1;
					uint8_t fz:1;
				};
				uint8_t f;
			};
			uint8_t a;
		};
		uint16_t af;
	};
	lr_dreg(b,c);
	lr_dreg(d,e);
	lr_dreg(h,l);
	uint16_t sp;
	uint16_t pc;
	uint8_t ifl;
	bool halted;
	// internal state
	lr35902_instruction_t op;
	// external
    busDevice_t* bus;
    void (*Reset)( struct lr35902_s *self );
    void (*Step)( struct lr35902_s *self );
} lr35902_t;


lr35902_t *Lr35902( busDevice_t *bus );