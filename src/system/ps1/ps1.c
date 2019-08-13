#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "../../device/device.h"

busDevice_t *Ps1IoMap( void ) {
    busDevice_t *iomap = GenericBus( "iomap" );

    // memory control
    GenericBusMapping( iomap, "Ex1BaseAddr", 0x0000, 0x0003,  GenericRegister( "Ex1BaseAddr", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "Ex2BaseAddr", 0x0004, 0x0007,  GenericRegister( "Ex2BaseAddr", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "Ex1DelSiz", 0x0008, 0x000b,  GenericRegister( "Ex1DelSize", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "Ex3DelSiz", 0x000c, 0x000f,  GenericRegister( "Ex3DelSize", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "BiosDelSiz", 0x0010, 0x0013,  GenericRegister( "BiosDelSize", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "SpuDelay", 0x0014, 0x0017,  GenericRegister( "SpuDelay", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "CdromDelay", 0x0018, 0x001b,  GenericRegister( "CdromDelay", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "Ex2DelSize", 0x001c, 0x001f,  GenericRegister( "Ex2DelSizee", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "ComDelay", 0x0020, 0x0023,  GenericRegister( "ComDelay", NULL, 4, NULL, NULL ) );

    // peripheral i/o
    GenericBusMapping( iomap, "JoyData", 0x0040, 0x0043,  GenericRegister( "JoyData", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "JoyStat", 0x0044, 0x0047,  GenericRegister( "JoyStat", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "JoyMode", 0x0048, 0x0049,  GenericRegister( "JoyMode", NULL, 2, NULL, NULL ) );
    GenericBusMapping( iomap, "JoyCtrl", 0x004a, 0x004b,  GenericRegister( "JoyCtrl", NULL, 2, NULL, NULL ) );
    GenericBusMapping( iomap, "JoyBaud", 0x004e, 0x004f,  GenericRegister( "JoyBaud", NULL, 2, NULL, NULL ) );
    GenericBusMapping( iomap, "SioData", 0x0050, 0x0053,  GenericRegister( "SioData", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "SioStat", 0x0054, 0x0057,  GenericRegister( "SioStat", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "SioMode", 0x0058, 0x0059,  GenericRegister( "SioMode", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "SioCtrl", 0x005a, 0x005b,  GenericRegister( "SioCtrl", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "SioMisc", 0x005c, 0x005d,  GenericRegister( "SioMisc", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "SioBaud", 0x005e, 0x005f,  GenericRegister( "SioBaud", NULL, 4, NULL, NULL ) );

    // memory control 2
    GenericBusMapping( iomap, "RamSize", 0x0060, 0x0063,  GenericRegister( "RamSize", NULL, 4, NULL, NULL ) );

    // interupt control

    GenericBusMapping( iomap, "IntStat", 0x0070, 0x0071,  GenericRegister( "IntStat", NULL, 2, NULL, NULL ) );
    GenericBusMapping( iomap, "IntMask", 0x0074, 0x0075,  GenericRegister( "IntMask", NULL, 2, NULL, NULL ) );

    // dma

    GenericBusMapping( iomap, "Dma0", 0x0080, 0x008f,  GenericRegister( "Dma0", NULL, 16, NULL, NULL ) );
    GenericBusMapping( iomap, "Dma1", 0x0090, 0x009f,  GenericRegister( "Dma1", NULL, 16, NULL, NULL ) );
    GenericBusMapping( iomap, "Dma2", 0x00a0, 0x00af,  GenericRegister( "Dma2", NULL, 16, NULL, NULL ) );
    GenericBusMapping( iomap, "Dma3", 0x00b0, 0x00bf,  GenericRegister( "Dma3", NULL, 16, NULL, NULL ) );
    GenericBusMapping( iomap, "Dma4", 0x00c0, 0x00cf,  GenericRegister( "Dma4", NULL, 16, NULL, NULL ) );
    GenericBusMapping( iomap, "Dma5", 0x00d0, 0x00df,  GenericRegister( "Dma5", NULL, 16, NULL, NULL ) );
    GenericBusMapping( iomap, "Dma6", 0x00e0, 0x00ef,  GenericRegister( "Dma6", NULL, 16, NULL, NULL ) );
    GenericBusMapping( iomap, "DPCR", 0x00f0, 0x00f3,  GenericRegister( "DPCR", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "DICR", 0x00f4, 0x00f7,  GenericRegister( "DICR", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "Unk1", 0x00f8, 0x00fb,  GenericRegister( "Unk1", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "Unk2", 0x00fc, 0x00ff,  GenericRegister( "Unk2", NULL, 4, NULL, NULL ) );

    // timers
    GenericBusMapping( iomap, "Timer0Dot", 0x0100, 0x010f,  GenericRegister( "Timer0Dot", NULL, 16, NULL, NULL ) );
    GenericBusMapping( iomap, "Timer1Hrf", 0x0110, 0x011f,  GenericRegister( "Timer1Hrf", NULL, 16, NULL, NULL ) );
    GenericBusMapping( iomap, "Timer2Sys", 0x0120, 0x012f,  GenericRegister( "Timer2Sys", NULL, 16, NULL, NULL ) );

    // cdrom
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );

    //gpu
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );

    //mdec
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );

    //spu voice
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );

    //spu control
    GenericBusMapping( iomap, "SpuMainVol", 0x0d80, 0x0d83,  GenericRegister( "SpuMainVol", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "SpuRvrbVol", 0x0d84, 0x0d87,  GenericRegister( "SpuRvrbVol", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );

    //spu reverb

    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );

    // spu internal
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );

    //expansion region 2 serial port
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "", 0x00, 0x00,  GenericRegister( "", NULL, 4, NULL, NULL ) );

    // expansion region 2 post
    GenericBusMapping( iomap, "AtconsStat", 0x1000, 0x1000,  GenericRegister( "AtconsStat", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "AtconsData", 0x1002, 0x1002,  GenericRegister( "AtconsData", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "Unk16bData", 0x1004, 0x1005,  GenericRegister( "Unk16bData", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "Irq10Flags", 0x1030, 0x1033,  GenericRegister( "Irq10Flags", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "UnkIrqCtrl", 0x1032, 0x1032,  GenericRegister( "UnkIrqCtrl", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "DipSwitchs", 0x1040, 0x1040,  GenericRegister( "DipSwitchs", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "Post1     ", 0x1041, 0x1041,  GenericRegister( "Post1     ", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "PostLED   ", 0x1042, 0x1042,  GenericRegister( "PostLED   ", NULL, 4, NULL, NULL ) );
    GenericBusMapping( iomap, "Post2     ", 0x1070, 0x1070,  GenericRegister( "Post2     ", NULL, 4, NULL, NULL ) );

    return iomap;
}

int main( int argc, char **argv ) {
    printf( "kutaragi! v%u.%u.%u%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_DIST );

    busDevice_t *ps1bus = GenericBus( "ps1" );
    busDevice_t *iomap = Ps1IoMap();

    busDevice_t* ram = GenericRam( 2 * 1024 * 1024 );
    busDevice_t* rom = GenericRom( "psx.rom", 512 * 1024 );
    busDevice_t* scratchpad = GenericRam( 1 * 1024 );
    
    // KUSEG mappings
    GenericBusMapping( ps1bus, "ram.kuseg",     0x00000000, 0x001fffff, ram );
    GenericBusMapping( ps1bus, "ram.kuseg",     0x00200000, 0x003fffff, ram );
    GenericBusMapping( ps1bus, "ram.kuseg",     0x00400000, 0x005fffff, ram );
    GenericBusMapping( ps1bus, "ram.kuseg",     0x00600000, 0x007fffff, ram );
    GenericBusMapping( ps1bus, "rom.kuseg",     0x1fc00000, 0x1fc7ffff, rom );
    GenericBusMapping( ps1bus, "scratch.kuseg", 0x1f800000, 0x1f8003ff, scratchpad );
    GenericBusMapping( ps1bus, "iomap.kuseg",   0x1f801000, 0x1f802fff, iomap );
    // KSEG0 mappings
    GenericBusMapping( ps1bus, "ram.kseg0",     0x80000000, 0x801fffff, ram );
    GenericBusMapping( ps1bus, "ram.kseg0",     0x80200000, 0x803fffff, ram );
    GenericBusMapping( ps1bus, "ram.kseg0",     0x80400000, 0x805fffff, ram );
    GenericBusMapping( ps1bus, "ram.kseg0",     0x80600000, 0x807fffff, ram );
    GenericBusMapping( ps1bus, "rom.kseg0",     0x9fc00000, 0x9fc7ffff, rom );
    GenericBusMapping( ps1bus, "scratch.kseg0", 0x9f800000, 0x9f8003ff, scratchpad );
    GenericBusMapping( ps1bus, "iomap.kseg0",   0x9f801000, 0x9f802fff, iomap );
    // KSEG1 mappings
    GenericBusMapping( ps1bus, "ram.kseg1",     0xa0000000, 0xa01fffff, ram );
    GenericBusMapping( ps1bus, "ram.kseg1",     0xa0200000, 0xa03fffff, ram );
    GenericBusMapping( ps1bus, "ram.kseg1",     0xa0400000, 0xa05fffff, ram );
    GenericBusMapping( ps1bus, "ram.kseg1",     0xa0600000, 0xa07fffff, ram );
    GenericBusMapping( ps1bus, "rom.kseg1",     0xbfc00000, 0xbfc7ffff, rom );
    GenericBusMapping( ps1bus, "iomap.kseg1",   0xbf801000, 0xbf802fff, iomap );
    GenericBusMapping( ps1bus, "CacheControl",   0xfffe0130, 0xfffe0133,  GenericRegister( "CacheControl", NULL, 4, NULL, NULL ) );

    // KSEG2 mappings
    // GenericBusMapping( ps1bus, "cache.kseg2",   0xfffe0000, 0xbf801fff, cachecontrol );
    mips1_t *cpu = Mips1( ps1bus );

    char *bytes = GenericRomBytesPtr( rom );
    if ( bytes != NULL ) {
        char romVer[256], romCopyright[256];
        strncpy( romVer, &bytes[0x42e74], 255 );
        strncpy( romCopyright, &bytes[0x42e8c], 255 );
        printf( "\n%s%s\n", romVer, romCopyright );
    }

    while ( true ) {
	    cpu->Step( cpu );
    }
}