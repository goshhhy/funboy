
typedef struct gbTimer_s {
    sm83_t *cpu;
    void (*Step)( struct gbTimer_s *self );
} gbTimer_t;

gbTimer_t *GbTimer( busDevice_t *bus, sm83_t *cpu );