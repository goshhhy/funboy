
#define LCDC_BITS_BG_ENABLE                 0x01
#define LCDC_BITS_OBJ_ENABLE                0x02
#define LCDC_BITS_OBJ_SIZE                  0x04
#define LCDC_BITS_BG_MAP_SELECT             0x08
#define LCDC_BITS_BG_WINDOW_DATA_SELECT     0x10
#define LCDC_BITS_WINDOW_ENABLE             0x20
#define LCDC_BITS_WINDOW_MAP_SELECT         0x40
#define LCDC_BITS_DISPLAY_ENABLE            0x80

#define OBJATTR_BITS_DMG_PALNUM             0x10
#define OBJATTR_BITS_X_FLIP                 0x20
#define OBJATTR_BITS_Y_FLIP                 0x40
#define OBJATTR_BITS_LAYER                  0x80

typedef struct gbPpu_s {
    sm83_t *cpu;
    busDevice_t *bgRam;
    busDevice_t *cRam;
    busDevice_t *oam;

    /* raw access */
    unsigned char *bgRamBytes;
    unsigned char *cRamBytes;
    unsigned char *oamBytes;

    /* alarms */
    alarmManager_t * alarmManager;
    alarm_t dmaAlarm, hsyncAlarm, vsyncAlarm;

    void (*Step)( struct gbPpu_s *self );
} gbPpu_t;

gbPpu_t *GbPpu( busDevice_t *bus, sm83_t *cpu, busDevice_t *bgRam, busDevice_t *cRam, busDevice_t *oam, alarmManager_t * alarmManager );
