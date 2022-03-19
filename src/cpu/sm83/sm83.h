#pragma pack(1)

typedef struct {
    void (*op)( void * cpu );
    unsigned char orig;
    unsigned char cycles;
} sm83_opcache_t;

typedef struct sm83_s { 
	unsigned char b, c, d, e, h, l, a, f, ifl, ifl_next;
	unsigned short sp, pc;
	/* internal state */ 
	unsigned char op;
    unsigned char lastop;
	int halted, fetched, timetarget;

    sm83_opcache_t *opcache;
	int cache_touched;

    /* external */
    busDevice_t* bus;
    void (*Reset)( struct sm83_s *self );
    void (*Step)( struct sm83_s *self );
    unsigned long (*StepMultiple)( struct sm83_s *self, unsigned long count, unsigned long *cycleProgress, int * stopFlag );
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

typedef enum {
    INTERPRETER_MODE_STANDARD,
    INTERPRETER_MODE_CACHED
} interpreterMode_t;

typedef struct {
    unsigned char op1;
    unsigned char op2;
    void (*impl)( sm83_t * cpu );
} sm83_fused_op_t;

extern sm83_fused_op_t sm83_fused_ops[];

void Sm83_SetInterpreterMode( sm83_t *cpu, interpreterMode_t mode );
void Sm83_InvalidateBankingRomCache( sm83_t * cpu );

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