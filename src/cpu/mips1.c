#include <stdio.h>
#include <stdint.h>

typedef struct mipsRegs_s {
    uint32_t gp_regs[32];
    uint32_t sr;
    struct {
        union {
            uint64_t mf;
            union {
                uint32_t mfhi;
                uint32_t mflo;
            };
        };
    };
    uint32_t pc;
} mipsRegs_t;

typedef struct mipsInstruction_s {
    union { 
        struct { //j-type
            uint8_t op : 6;
            uint32_t target : 26;
        };
        struct { //i-type
            uint8_t op_i : 6;
            uint8_t rs : 5;
            uint8_t rt : 5;
            uint16_t imm;
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

static void Exception( void ) {

}

static void RunInstruction( void ) {

}