
typedef struct gbTimer_s {
    sm83_t *cpu;
    void (*Step)( struct gbTimer_s *self );
    alarmManager_t * alarmManager;
    alarm_t alarm;
} gbTimer_t;

gbTimer_t *GbTimer( busDevice_t *bus, sm83_t *cpu, alarmManager_t * alarmManager );