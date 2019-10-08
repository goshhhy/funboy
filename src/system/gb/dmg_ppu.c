#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "../../version.h"
#include "../../device/device.h"
#include "../../cpu/sm83/sm83.h"
#include "../../io/io.h"
#include "ppu.h"

// register values
static uint8_t lcdc;
static uint8_t lcdStat;
static uint8_t scx, scy, wx, wy;
static uint8_t ly, lyc;
static uint8_t bgPal, objPal0, objPal1;
static int16_t dotClock, dotDelay, dotDelayTotal;

static bool enabled;

uint8_t colors[4][3] = {
	{ 0xe7, 0xe7, 0xc7 },
	{ 0xc3, 0xc3, 0xa3 },
	{ 0x9b, 0x9b, 0x8b },
	{ 0x38, 0x38, 0x28 },
};

typedef struct regInfo_s {
    char* name;
    size_t len;
    uint32_t* data;
} regInfo_t;

static void ControlRegisterWrite( busDevice_t *dev, uint32_t addr, uint8_t val, bool final ) {
    regInfo_t *reg;

    if ( !dev || !dev->data )
        return;
    reg = dev->data;

    if ( addr > reg->len ) {
        fprintf( stderr, "warning: DivRegisterWrite: address out of bounds\n" );
        return;
    }

    printf( "write register [0x%08x]%s <- %02x (byte %u)\n", addr, reg->name, val, addr );
    fprintf( stderr, "write register [0x%08x]%s <- %02x (byte %u)\n", addr, reg->name, val, addr );
    lcdc = val;

    enabled = ( ( lcdc & 0x80 ) != 0 );
    if ( enabled ) {
        fprintf( stderr, "lcd enabled\n" );
    } else {
        if ( ly < 144 ) {
            fprintf( stderr, "warning: lcd disabled outside of vblank!\n" );
        } else {
            fprintf( stderr, "lcd disabled\n" );
        }
        ly = 0;
        dotClock = dotDelay = dotDelayTotal = 0;
    }
}

static void Step( gbPpu_t *self ) {
    uint8_t shade;
    uint16_t tileDataBase = 0x0000, bgMapBase= 0x0000;

    if ( !enabled )
        return;

    lcdStat = lcdStat & 0xfc;

    if ( dotDelay > 0 ) {
        dotDelay--;
        return;
    }

    if ( ( dotClock < 160 ) && ( ly < 144 ) ) {
        if ( ( lcdc & 0x10 ) == 0 ) {
            tileDataBase += 0x800;
        }
        if ( ( lcdc & 0x08 ) == 1 ) {
            bgMapBase += 0x400;
        }
        // calculate bg pixel
        uint8_t bgLine = scy + ly;
        uint8_t bgRow = scx + dotClock;
        uint8_t tileLine = bgLine / 8;
        uint8_t tileRow = bgRow / 8;
        uint16_t tileOffset = ( (uint16_t)tileLine * (uint16_t)32 ) + (uint16_t)tileRow;
        uint8_t tileNum = self->bgRam->Read8( self->bgRam, bgMapBase + tileOffset, false ); 

        if ( ( lcdc & 0x10 ) == 0 ) {
            int8_t tileFix = (int8_t)tileNum;
            tileFix = tileFix + 128;
            tileNum = tileFix;
        }

        uint8_t pixLine = bgLine % 8;
        uint8_t pixRow = bgRow % 8;
        uint16_t tileByteIndex = pixLine * 2;

        uint8_t upperByte = self->bgRam->Read8( self->cRam, tileDataBase + ( tileNum * 16 ) + tileByteIndex, false );
        uint8_t lowerByte = self->bgRam->Read8( self->cRam, tileDataBase + ( tileNum * 16 ) + tileByteIndex + 1, false );
        uint8_t palIndex = ( ( upperByte >> ( 7 - pixRow ) ) & 0x1 );
        palIndex |= ( ( lowerByte >> ( 7 - pixRow ) ) & 0x1 ) << 1;
        shade = ( bgPal >> ( 2 * palIndex ) ) & 0x03;

        IO_DrawPixel( dotClock, ly, colors[shade][0], colors[shade][1], colors[shade][2] );
        lcdStat |= 0x03;
    } else if ( ly < 144 ) {
        if ( dotClock == 160 ) {
            if ( ( lcdStat & 0x08 ) != 0 ) {
                self->cpu->Interrupt( self->cpu, 1 );
            }
        }
        lcdStat |= 0x00;
    } else {
        lcdStat |= 0x01;
    }

    dotClock++;
    if ( dotClock > 376 ) {
        dotClock = 0;
        ly++;
        if ( ly > 153 ) {
            ly = 0;
        }
        if ( ly == 144 ) {
            self->cpu->Interrupt( self->cpu, 0 );
            if ( ( lcdStat & 0x10 ) != 0 ) {
                self->cpu->Interrupt( self->cpu, 1 );
            }
        }
        if ( ly == lyc )  {
            if ( ( lcdStat & 0x40 ) != 0 ) {
                self->cpu->Interrupt( self->cpu, 1 );
            }
            lcdStat = lcdStat | 0x04;
        } else {
            lcdStat = lcdStat & 0xfb;
        }

    }
}

gbPpu_t *GbPpu( busDevice_t *bus, sm83_t *cpu, busDevice_t *bgRam, busDevice_t *cRam ) {
    gbPpu_t *ppu = malloc( sizeof( gbPpu_t ) );
    ppu->Step = Step;
    ppu->cpu = cpu;
    ppu->bgRam = bgRam;
    ppu->cRam = cRam;
    IO_Init( 640, 576, 160, 144 );
    IO_SetBg( 0xf0, 0xf0, 0xd0 );

    enabled = true;
    lcdc = lcdStat = 0;
    scy = scx = wy = wx = 0;
    bgPal = objPal0 = objPal1 = 0;

    GenericBusMapping( bus, "LCDC", 0xFF40, 0xFF40, GenericRegister( "LCDC", &lcdc, 1, NULL, ControlRegisterWrite ) );
    GenericBusMapping( bus, "LCDStat", 0xFF41, 0xFF41, GenericRegister( "LCDStat", &lcdStat, 1, NULL, NULL ) );
    GenericBusMapping( bus, "SCY", 0xFF42, 0xFF42, GenericRegister( "SCY", &scy, 1, NULL, NULL ) );
    GenericBusMapping( bus, "SCX", 0xFF43, 0xFF43, GenericRegister( "SCX", &scx, 1, NULL, NULL ) );
    GenericBusMapping( bus, "LY", 0xFF44, 0xFF44, GenericRegister( "LY", &ly, 1, NULL, NULL ) );
    GenericBusMapping( bus, "LYC", 0xFF45, 0xFF45, GenericRegister( "LYC", &lyc, 1, NULL, NULL ) );
    GenericBusMapping( bus, "BGP", 0xFF47, 0xFF47, GenericRegister( "BGP", &bgPal, 1, NULL, NULL ) );
    GenericBusMapping( bus, "OBP0", 0xFF48, 0xFF48, GenericRegister( "OBP0", &objPal0, 1, NULL, NULL ) );
    GenericBusMapping( bus, "OBP1", 0xFF49, 0xFF49, GenericRegister( "OBP1", &objPal1, 1, NULL, NULL ) );
    GenericBusMapping( bus, "WY", 0xFF4A, 0xFF4A, GenericRegister( "WY", &wy, 1, NULL, NULL ) );
    GenericBusMapping( bus, "WX", 0xFF4B, 0xFF4B, GenericRegister( "WX", &wx, 1, NULL, NULL ) );

    return ppu;
}
