
typedef struct gbInput_s {
    sm83_t *cpu;
} gbInput_t;

gbInput_t *GbInput( busDevice_t *bus, sm83_t *cpu );