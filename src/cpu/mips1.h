// contains all data needed for mips processor emulation, and callback functions
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
    void (*Reset)( struct mips1_s *self );
    void (*Step)( struct mips1_s *self );
} mips1_t;

mips1_t *Mips1( busDevice_t *bus );