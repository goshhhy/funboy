#pragma pack(1)

typedef union sm83_instruction_s {
	struct {
		unsigned int b0:1;
		unsigned int b1:1;
		unsigned int b2:1;
		unsigned int b3:1;
		unsigned int b4:1;
		unsigned int b5:1;
		unsigned int b6:1;
		unsigned int b7:1;
	} bits;
	struct {
		unsigned int z:3;
		unsigned int y:3;
		unsigned int x:2;
	} xyz;
	struct {
		unsigned int z2:3;
		unsigned int q:1;
		unsigned int p:2;
		unsigned int x2:2;
	} pqxz;
	unsigned char full;
} sm83_instruction_t;

typedef struct sm83_s { 
	union{
		struct{
			unsigned int fl:4;
			unsigned int fc:1;
			unsigned int fh:1;
			unsigned int fn:1;
			unsigned int fz:1;
		} flags;
		unsigned char reg;
	} f;
	unsigned char a, b, c, d, e, h, l, ifl;
	unsigned short sp, pc;
	int halted;
	/* internal state */ 
	sm83_instruction_t op;
	int fetched;
	int timetarget;
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

void Step_Cached( sm83_t *cpu );

unsigned char read_r( sm83_t *cpu, unsigned char r );
void write_r( sm83_t *cpu, unsigned char val, unsigned char r );
unsigned short read_rp( sm83_t *cpu, unsigned char r );
void write_rp( sm83_t *cpu, unsigned short val, unsigned char r );
unsigned short read_rp2( sm83_t *cpu, unsigned char r );
void write_rp2( sm83_t *cpu, unsigned short val, unsigned char r );
void pushw( sm83_t* cpu, unsigned short word );
unsigned short popw( sm83_t* cpu );

#pragma pack(0)