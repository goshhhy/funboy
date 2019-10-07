#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "../../version.h"
#include "../../device/device.h"
#include "../../cpu/sm83/sm83.h"
#include "timer.h"

#define REGISTER( dev, name, where, size ) GenericBusMapping( dev, name, where, where + size - 1,  GenericRegister( name, NULL, size, NULL, NULL ) );

#define GB_CLOCK_SPEED 4194304

void MapGbRegs( busDevice_t* gbbus ) {
    REGISTER( gbbus, "JoyIn",           0xFF00, 1 );

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

    REGISTER( gbbus, "LCDControl",      0xFF40, 1 );
    REGISTER( gbbus, "LCDStat",         0xFF41, 1 );
    REGISTER( gbbus, "LCDScrollY",      0xFF42, 1 );
    REGISTER( gbbus, "LCDScrollX",      0xFF43, 1 );
    REGISTER( gbbus, "LCDY",            0xFF44, 1 );
    REGISTER( gbbus, "LYComp"    ,      0xFF45, 1 );
    REGISTER( gbbus, "DMATx",           0xFF46, 1 );
    REGISTER( gbbus, "BGPal",           0xFF47, 1 );
    REGISTER( gbbus, "ObjPal0",         0xFF48, 1 );
    REGISTER( gbbus, "ObjPal1",         0xFF49, 1 );
    REGISTER( gbbus, "WinY",            0xFF4A, 1 );
    REGISTER( gbbus, "WinX",            0xFF4B, 1 );
    
    REGISTER( gbbus, "IntFlag",         0xFF0F, 1 );
    REGISTER( gbbus, "IntEnable",       0xFFFF, 1 );
}

int main( int argc, char **argv ) {
    int framestep = 0;
    printf( "kutaragi!gb v%u.%u.%u%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_DIST );

    busDevice_t *gbbus = GenericBus( "gb" );

    busDevice_t* ram = GenericRam( 0x2000 );
    busDevice_t* zpage = GenericRam( 0x7f );
    busDevice_t* cram = GenericRam( 0x17ff );
    busDevice_t* bgram = GenericRam( 0x800 );
    busDevice_t* oam = GenericRam( 0x9f );
    busDevice_t* rom = GenericRom( argv[1], 32 * 1024 );
    busDevice_t* cartram = GenericRam( 0x2000 );
    
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

    sm83_t *cpu = Sm83( gbbus );
    gbTimer_t *timer = GbTimer( gbbus, cpu );

    while ( true ) {
        for ( framestep = 0; framestep < GB_CLOCK_SPEED / 60; framestep++ ) {
            cpu->Step( cpu );
            timer->Step( timer );
        }
    }
}