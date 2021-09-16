#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "version.h"
#include "device.h"
#include "sm83.h"
#include "alarms.h"
#include "timer.h"
#include "gb.h"

static int enabled;
static unsigned char divReg;
static unsigned long divTimeOffset;
static unsigned char divSubcount;
static unsigned char countReg;
static unsigned char countSubcount;
static unsigned char divisor;
static unsigned char modulo;
static unsigned char control;
static int overflowed;

static unsigned long NextTime( unsigned long start, unsigned long now, unsigned long interval ) {
    unsigned long diff = now - start;
    return start + interval + interval * ( diff / interval );
}

static void SetNextTimerAlarm( gbTimer_t *self, int inCallback ) {
    int cycles;
    unsigned long now;
    unsigned long next;
    unsigned long timePassed;

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

    timePassed = self->alarmManager->AlarmGetTimePassedCallback(self->alarmManager->alarmGetTimePassedCallbackData);

    now = self->alarmManager->AlarmGetFrameTimeCallback(self->alarmManager->alarmGetFrameTimeCallbackData) + 1;
    now += timePassed;

    next = NextTime( divTimeOffset, now, cycles ) - now;

    /*printf( "timer alarm set: %i cycle interval, fires in %i cycles at %i (start %i now %i timepassed %i) %s\n", cycles, next, now + next, divTimeOffset, now, timePassed, inCallback ? "(callback)" : "" );*/

    if ( !inCallback ) {
        self->alarm.when = next + timePassed;
    } else {
        self->alarm.when = next;
    }

    self->alarmManager->AlarmChangedCallback(self->alarmManager->alarmChangedCallbackData);
}

static unsigned char DivRegisterRead( busDevice_t *dev, busAddress_t addr, int final ) {
    regInfo_t * reg;
    gbTimer_t * self;
    unsigned long globaltime;

    if ( !dev || !dev->data )
        return 0;
    reg = dev->data;
    self = reg->data;

    globaltime = self->alarmManager->AlarmGetFrameTimeCallback(self->alarmManager->alarmGetFrameTimeCallbackData);

    divReg = (globaltime % 256) + (divTimeOffset % 256);
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
    globaltime += self->alarmManager->AlarmGetTimePassedCallback(self->alarmManager->alarmGetTimePassedCallbackData) + 1;

    divReg = 0;
    divSubcount = 0;
    divTimeOffset = globaltime;

    /*printf("div register written at %lu\n", globaltime);*/

    SetNextTimerAlarm( self, 0 );
}

static unsigned char ControlRegisterRead( busDevice_t *dev, busAddress_t addr, int final ) {
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

    /*printf( "write register [0x%04x]%s <- %02hx (byte %hu)\n", addr, reg->name, val, addr );*/
    control = val & 0x07;

    enabled = ( ( control & 4 ) != 0 );

    SetNextTimerAlarm( self, 0 );

    fprintf( stderr, "timer set! ctrl %02x\n", control & 0x03 );
}

/* no longer used */
static void Step( gbTimer_t *self ) {
    (void)self;
}

static void AlarmTimerCallback( void * data ) {
    gbTimer_t * self = data;
    unsigned long now;

    countReg++;

    if ( enabled && overflowed ) {
        self->cpu->Interrupt( self->cpu, 2 );
        countReg = modulo;
        overflowed = 0;
    } 
    
    if ( countReg == 0 ) {
        overflowed = 1;
    }

    now = self->alarmManager->AlarmGetFrameTimeCallback(self->alarmManager->alarmGetFrameTimeCallbackData);
    now += self->alarmManager->AlarmGetTimePassedCallback(self->alarmManager->alarmGetTimePassedCallbackData) + 1;

    /*printf("timer fired at %lu\n", now);*/

    SetNextTimerAlarm( self, 1 );
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

    SetNextTimerAlarm( timer, 1 );

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

