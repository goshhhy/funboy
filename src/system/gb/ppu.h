
typedef struct gbPpu_s {
    sm83_t *cpu;
    busDevice_t *bgRam;
    busDevice_t *cRam;
    busDevice_t *oam;
    /* raw access */
    unsigned char *bgRamBytes;
    unsigned char *cRamBytes;
    unsigned char *oamBytes;

    void (*Step)( struct gbPpu_s *self );
} gbPpu_t;

gbPpu_t *GbPpu( busDevice_t *bus, sm83_t *cpu, busDevice_t *bgRam, busDevice_t *cRam, busDevice_t *oam  );