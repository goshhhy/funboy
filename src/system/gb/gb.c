#include <stdio.h>
#include <string.h>

#include "version.h"
#include "device.h"
#include "sm83.h"
#include "io.h"
#include "alarms.h"
#include "gb.h"

#include "timer.h"
#include "ppu.h"
#include "input.h"
#include "mbc.h"
#include "serial_log.h"

#define REGISTER( dev, name, where, size ) GenericBusMapping( dev, name, where, where + size - 1,  GenericRegister( name, NULL, size, NULL, NULL ) );

void MapGbRegs( busDevice_t* gbbus );
busDevice_t *LoadRom( char* path, busDevice_t** cartram, alarmManager_t *alarmManager, sm83_t * cpu );

void MapGbRegs( busDevice_t* gbbus ) {
    REGISTER( gbbus, "SndCh1Sweep",     0xFF10, 1 );
    REGISTER( gbbus, "SndCh1Len",       0xFF11, 1 );
    REGISTER( gbbus, "SndCh1Vol",       0xFF12, 1 );
    REGISTER( gbbus, "SndCh1FrqL",      0xFF13, 1 );
    REGISTER( gbbus, "SndCh1FrqH",      0xFF14, 1 );
    REGISTER( gbbus, "SndCh2Len",       0xFF16, 1 );
    REGISTER( gbbus, "SndCh2Vol",       0xFF17, 1 );
    REGISTER( gbbus, "SndCh2FrqL",      0xFF18, 1 );
    REGISTER( gbbus, "SndCh2FrqH",      0xFF19, 1 );
    REGISTER( gbbus, "SndCh3On",        0xFF1A, 1 );
    REGISTER( gbbus, "SndCh3Len",       0xFF1B, 1 );
    REGISTER( gbbus, "SndCh3Lvl",       0xFF1C, 1 );
    REGISTER( gbbus, "SndCh3FrqL",      0xFF1D, 1 );
    REGISTER( gbbus, "SndCh3FrqH",      0xFF1E, 1 );
    REGISTER( gbbus, "SndCh4Len",       0xFF20, 1 );
    REGISTER( gbbus, "SndCh4Vol",       0xFF21, 1 );
    REGISTER( gbbus, "SndCh4Ctr",       0xFF22, 1 );
    REGISTER( gbbus, "SndCh4Cons",      0xFF23, 1 );

    REGISTER( gbbus, "SndVol",          0xFF24, 1 );
    REGISTER( gbbus, "SndOut",          0xFF25, 1 );
    REGISTER( gbbus, "SndOn",           0xFF26, 1 );

    REGISTER( gbbus, "WavRam0",         0xFF30, 1 );
    REGISTER( gbbus, "WavRam1",         0xFF31, 1 );
    REGISTER( gbbus, "WavRam2",         0xFF32, 1 );
    REGISTER( gbbus, "WavRam3",         0xFF33, 1 );
    REGISTER( gbbus, "WavRam4",         0xFF34, 1 );
    REGISTER( gbbus, "WavRam5",         0xFF35, 1 );
    REGISTER( gbbus, "WavRam6",         0xFF36, 1 );
    REGISTER( gbbus, "WavRam7",         0xFF37, 1 );
    REGISTER( gbbus, "WavRam8",         0xFF38, 1 );
    REGISTER( gbbus, "WavRam9",         0xFF39, 1 );
    REGISTER( gbbus, "WavRam10",        0xFF3A, 1 );
    REGISTER( gbbus, "WavRam11",        0xFF3B, 1 );
    REGISTER( gbbus, "WavRam12",        0xFF3C, 1 );
    REGISTER( gbbus, "WavRam13",        0xFF3D, 1 );
    REGISTER( gbbus, "WavRam14",        0xFF3E, 1 );
    REGISTER( gbbus, "WavRam15",        0xFF3F, 1 );

    REGISTER( gbbus, "IntFlag",         0xFF0F, 1 );
    REGISTER( gbbus, "IntEnable",       0xFFFF, 1 );
}

busDevice_t *LoadRom( char* path, busDevice_t** cartram, alarmManager_t* alarmManager, sm83_t * cpu ) {
    unsigned char romName[17] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
    busDevice_t* rom = GenericRom( path, 512 );
    unsigned char mapperType;
    size_t romSize;
    int i;

    if ( !rom )
        return NULL;

    for ( i = 0; i < 16; i++ )
        romName[i] = rom->Read8( rom, 0x134 + i, 0 );

    if ( ( romName[15] == 0x80 ) || ( romName[15 ] == 0xC0 ) ) {
        romName[15] = '\0';        
    } 
    printf( "Loading rom %s\n", romName );
    IO_SetTitle( (const char*)romName );

    mapperType = rom->Read8( rom, 0x147, 0 );
    romSize = ( 32 * 1024 ) << rom->Read8( rom, 0x148, 0 );
    printf( "mapper type is %02x\n", rom->Read8( rom, 0x147, 0 ) );
    printf( "rom size is %ukb\n", (int)(romSize / 1024) );

    switch( mapperType ) {
        case 0x01:
        case 0x02:
        case 0x03:
            rom = MBC1Rom( path, romSize, cartram, alarmManager, cpu );
            break;
        case 0x11:
        case 0x12:
        case 0x13:
            rom = MBC3Rom( path, romSize, cartram, alarmManager, cpu );
            break;
        default:
            rom = GenericRom( path, romSize );
            *cartram = GenericRam( 0x2000, "cartram" );
    }
    return rom;
}

static void AlarmChangedCallback(void * data) {
    int * alarmChanged = data;
    
    *alarmChanged = 1;
}

static int AlarmGetTimePassedCallback(void * data) {
    int * timePassed = data;
    
    return *timePassed;
}

static unsigned long AlarmGetFrameTimeCallback(void * data) {
    int * frameTime = data;
    
    return *frameTime;
}

int gb_main( char *rompath ) {
    unsigned long framestep = 0;
    unsigned long substep = 0;
    int go = 1;
    int alarmChanged = 0;
    busDevice_t *gbbus;
    busDevice_t* ram;
    busDevice_t* zpage;
    busDevice_t* cram;
    busDevice_t* bgram;
    busDevice_t* oam;
    busDevice_t* cartram;
    busDevice_t* rom;
    sm83_t *cpu;
    gbTimer_t *timer;
    gbPpu_t *ppu;
    alarmManager_t *alarmManager;

    printf( "funboy! v%u.%u.%u%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_DIST );

    alarmManager = AlarmManager( AlarmChangedCallback, &alarmChanged );
    alarmManager->AlarmGetTimePassedCallback = AlarmGetTimePassedCallback;
    alarmManager->alarmGetTimePassedCallbackData = &substep;
    alarmManager->AlarmGetFrameTimeCallback = AlarmGetFrameTimeCallback;
    alarmManager->alarmGetFrameTimeCallbackData = &framestep;


    gbbus = GenericBus( "gb" );

    cpu = Sm83( gbbus );

    ram = GenericRam( 0x2000, "ram" );
    zpage = GenericRam( 0x80, "zpage" );
    cram = GenericRam( 0x1800, "cram" );
    bgram = GenericRam( 0x800, "bgram" );
    oam = GenericRam( 0xa0, "oam" );

    rom = LoadRom( rompath, &cartram, alarmManager, cpu );

    if ( !rom ) {
        return 1;
    }

    GenericBusMapping( gbbus, "rom",     0x0000, 0x7fff, rom );
    GenericBusMapping( gbbus, "cram",    0x8000, 0x97ff, cram );
    GenericBusMapping( gbbus, "bgram",   0x9800, 0x9fff, bgram );
    GenericBusMapping( gbbus, "cartram", 0xa000, 0xbfff, cartram );
    GenericBusMapping( gbbus, "ram",     0xc000, 0xdfff, ram );
    GenericBusMapping( gbbus, "echo",    0xe000, 0xfdff, ram );
    GenericBusMapping( gbbus, "oam",     0xfe00, 0xfe9f, oam );
    GenericBusMapping( gbbus, "zpage",   0xff80, 0xfffe, zpage );

    MapGbRegs( gbbus );
    SerialToStderr( gbbus );

    timer = GbTimer( gbbus, cpu, alarmManager );
    GbInput( gbbus, cpu );

    /* init ppu last, since it sets up the benchmark timer */
    ppu = GbPpu( gbbus, cpu, bgram, cram, oam, alarmManager );
    IO_SetEmuName( "funboy!" );

    Sm83_SetInterpreterMode( cpu, INTERPRETER_MODE_CACHED );

    while ( go ) {
        for ( framestep = 0; framestep < (GB_CLOCK_SPEED / 59.97) - 1; ) {
            alarmChanged = 0;
            alarm_t * next = AlarmGetNext( alarmManager );
            int target = ( next && ( next->when > 0 ) ) ? next->when : GB_CLOCK_SPEED / 59.97;
            
            cpu->StepMultiple( cpu, target, &substep, &alarmChanged );
            
            AlarmTimePassed( alarmManager, substep, 1 );
            framestep += substep;
        }
        go = IO_Update( cpu );
    }
    return 0;
}