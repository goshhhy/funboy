// contains all data needed for mips processor emulation, and callback functions

#pragma pack()
typedef struct mips1_s {
    uint32_t gp_regs[32];
    union {
       uint32_t sr;
       struct {
           uint8_t cu3: 1;
           uint8_t cu2: 1;
           uint8_t cu1: 1;
           uint8_t cu0: 1;
           uint8_t zero0: 2;
           uint8_t re: 1;
           uint8_t zero1: 2;
           uint8_t bev: 1;
           uint8_t ts: 1;
           uint8_t pe: 1;
           uint8_t cm: 1;
           uint8_t pz: 1;
           uint8_t swc: 1;
           uint8_t isc: 1;
           uint8_t im: 8;
           uint8_t zero2: 2;
           uint8_t kuo: 1;
           uint8_t ieo: 1;
           uint8_t kup: 1;
           uint8_t iep: 1;
           uint8_t kuc: 1;
           uint8_t iec: 1;
       };
    };
    uint32_t pc;
    union {
        uint64_t mf;
        union {
            uint32_t mfhi;
            uint32_t mflo;
        };
    };
    busDevice_t* bus;
    uint32_t steps;
    void (*Reset)( struct mips1_s *self );
    void (*Step)( struct mips1_s *self );
} mips1_t;

#pragma pack(0)
typedef struct mipsInstruction_s {
        union {
            uint32_t full;
            struct { //j-type
                uint8_t op : 6;
                uint32_t target : 26;
            };
            struct { //i-type
                uint8_t op_i : 6;
                uint8_t rs : 5;
                uint8_t rt : 5;
                uint16_t imm : 16;
            };
            struct { //r-type
                uint8_t op_r : 6;
                uint8_t rs_r : 5;
                uint8_t rt_r : 5;
                uint8_t rd : 5;
                uint8_t shamt : 5;
                uint8_t funct : 6; 
            };
        };
} mipsInstruction_t;
#pragma pack()

mips1_t *Mips1( busDevice_t *bus );