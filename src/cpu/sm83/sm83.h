#pragma pack(1)

typedef struct sm83_s { 
	unsigned char b, c, d, e, h, l, a, f, ifl, ifl_next;
	unsigned short sp, pc;
	/* internal state */ 
	unsigned char op;
	int halted, fetched, timetarget;
	/* external */
    busDevice_t* bus;
    void (*Reset)( struct sm83_s *self );
    void (*Step)( struct sm83_s *self );
	void (*Interrupt)( struct sm83_s *cpu, unsigned char inum );
} sm83_t;

typedef enum {
    LD_MODE_B  = 0,
    LD_MODE_C  = 1,
    LD_MODE_D  = 2,
    LD_MODE_E  = 3,
    LD_MODE_H  = 4,
    LD_MODE_L  = 5,
    LD_MODE_HL = 6,
    LD_MODE_A  = 7
} ldMode_t;

sm83_t *Sm83( busDevice_t *bus );
unsigned char read_r( sm83_t *cpu, unsigned char r );
void write_r( sm83_t *cpu, unsigned char val, unsigned char r );
unsigned short read_rp( sm83_t *cpu, unsigned char r );
void write_rp( sm83_t *cpu, unsigned short val, unsigned char r );
unsigned short read_rp2( sm83_t *cpu, unsigned char r );
void write_rp2( sm83_t *cpu, unsigned short val, unsigned char r );
void pushw( sm83_t* cpu, unsigned short word );
unsigned short popw( sm83_t* cpu );

#pragma pack(0)