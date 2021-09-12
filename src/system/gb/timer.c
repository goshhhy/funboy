#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "version.h"
#include "device.h"
#include "sm83.h"
#include "alarms.h"
#include "timer.h"
#include "gb.h"

typedef struct regInfo_s {
    char* name;
    size_t len;
    unsigned long* data;
} regInfo_t;

static int enabled;
static unsigned char divReg;
static unsigned char divTimeOffset;
static unsigned char divSubcount;
static unsigned char countReg;
static unsigned char countSubcount;
static unsigned char divisor;
static unsigned char modulo;
static unsigned char control;
static int interrupt_wait;
static int overflowed;


static int DivRegisterRead( busDevice_t *dev, busAddress_t addr, int final ) {
    regInfo_t * reg;
    gbTimer_t * self;
    unsigned long globaltime;

    if ( !dev || !dev->data )
        return 0;
    reg = dev->data;
    self = reg->data;

    globaltime = self->alarmManager->AlarmGetFrameTimeCallback(self->alarmManager->alarmGetFrameTimeCallbackData);

    divReg = (globaltime % 256) + divTimeOffset;
    return divReg;
}

static void DivRegisterWrite( busDevice_t *dev, busAddress_t addr, unsigned char val, int final ) {
    regInfo_t * reg;
    gbTimer_t * self;
    unsigned long globaltime;

    if ( !dev || !dev->data )
        return;
    reg = dev->data;
    self = reg->data;

    globaltime = self->alarmManager->AlarmGetFrameTimeCallback(self->alarmManager->alarmGetFrameTimeCallbackData);

    divReg = 0;
    divSubcount = 0;
    divTimeOffset = globaltime % 256;
}

static void SetNextTimerAlarm( gbTimer_t *self ) {
    int cycles;

    if ( !enabled ) {
        self->alarm.when = -1;
        self->alarmManager->AlarmChangedCallback(self->alarmManager->alarmChangedCallbackData);
        return;
    }

    switch ( control & 0x03 ) {
        case 0: cycles = GB_CLOCK_SPEED / 4096; break;
        case 1: cycles = GB_CLOCK_SPEED / 262144; break;
        case 2: cycles = GB_CLOCK_SPEED / 65536; break;
        case 3: cycles = GB_CLOCK_SPEED / 16384; break;
    }

    cycles += self->alarmManager->AlarmGetTimePassedCallback(self->alarmManager->alarmGetTimePassedCallbackData);
    self->alarm.when = cycles;
    
    self->alarmManager->AlarmChangedCallback(self->alarmManager->alarmChangedCallbackData);
}

static int ControlRegisterRead( busDevice_t *dev, busAddress_t addr, int final ) {
    return control;
}

static void ControlRegisterWrite( busDevice_t *dev, busAddress_t addr, unsigned char val, int final ) {
    regInfo_t * reg;
    gbTimer_t * self;

    if ( !dev || !dev->data )
        return;
    reg = dev->data;
    self = reg->data;

    if ( addr > reg->len ) {
        fprintf( stderr, "warning: DivRegisterWrite: address out of bounds\n" );
        return;
    }

    printf( "write register [0x%08lx]%s <- %02hx (byte %lu)\n", addr, reg->name, val, addr );
    control = val & 0x07;

    enabled = ( ( control & 4 ) != 0 );

    SetNextTimerAlarm( self );

    fprintf( stderr, "timer set! ctrl %02x\n", control & 0x03 );
}

/* no longer used */
static void Step( gbTimer_t *self ) {
    (void)self;
}

static void AlarmTimerCallback( void * data ) {
    gbTimer_t * self = data;

    if ( enabled && overflowed ) {
        printf( "timer interrupt!\n" );
        self->cpu->Interrupt( self->cpu, 2 );
        countReg = modulo;
        overflowed = 0;
    } 

    countReg++;
    
    if ( countReg == 0 ) {
        overflowed = 1;
    }

    SetNextTimerAlarm( self );
}

gbTimer_t *GbTimer( busDevice_t *bus, sm83_t *cpu, alarmManager_t * alarmManager ) {
    gbTimer_t *timer = malloc( sizeof( gbTimer_t ) );
    timer->Step = Step;
    timer->cpu = cpu;
    
    timer->alarm.name = "timerAlarm";
    timer->alarm.when = -1;
    timer->alarm.Callback = AlarmTimerCallback;
    timer->alarm.callbackData = timer;

    timer->alarmManager = alarmManager;

    AlarmAdd( alarmManager, &timer->alarm );

    SetNextTimerAlarm( timer );

    enabled = 0;
    divReg = 0;
    divSubcount = 0;
    countReg = 0;
    countSubcount = 0;
    divisor = 64;
    modulo = 0;

    GenericBusMapping( bus, "TmrDiv", 0xFF04, 0xFF04, GenericRegister( "TmrDiv", timer, 1, DivRegisterRead, DivRegisterWrite ) );
    GenericBusMapping( bus, "TmrCount", 0xFF05, 0xFF05, GenericRegister( "TmrCount", &countReg, 1, NULL, NULL ) );
    GenericBusMapping( bus, "TmrMod", 0xFF06, 0xFF06, GenericRegister( "TmrMod", &modulo, 1, NULL, NULL ) );
    GenericBusMapping( bus, "TmrCtrl", 0xFF07, 0xFF07, GenericRegister( "TmrCtrl", timer, 1, ControlRegisterRead, ControlRegisterWrite ) );
    return timer;
}

