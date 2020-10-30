#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../version.h"
#include "../../device/device.h"
#include "../../cpu/sm83/sm83.h"
#include "../../io/io.h"
#include "ppu.h"

/*  register values */
static unsigned char lcdc;
static unsigned char lcdStat;
static unsigned char scx, scy, wx, wy;
static unsigned char ly, lyc;
static unsigned char bgPal, objPal0, objPal1;
static short dotClock, dotDelay, dotDelayTotal;
static int dmaTransferActive = 0;
static unsigned short dmaAddress;
 
static int enabled;

unsigned char colors[4][3] = {
	{ 0xe7, 0xe7, 0xc7 },
	{ 0xc3, 0xc3, 0xa3 },
	{ 0x9b, 0x9b, 0x8b },
	{ 0x38, 0x38, 0x28 },
};

typedef struct regInfo_s {
    char* name;
    size_t len;
    unsigned long* data;
} regInfo_t;


static void DmaRegisterWrite( busDevice_t *dev, unsigned long addr, unsigned char val, int final ) {
    dmaTransferActive = 1;
    dmaAddress = (unsigned short)val << 8;
}

static void ControlRegisterWrite( busDevice_t *dev, unsigned long addr, unsigned char val, int final ) {
    regInfo_t *reg;
    int x, y;

    if ( !dev || !dev->data )
        return;
    reg = dev->data;

    if ( addr > reg->len ) {
        fprintf( stderr, "warning: DivRegisterWrite: address out of bounds\n" );
        return;
    }

    /* printf( "write register [0x%08x]%s <- %02x (byte %u)\n", addr, reg->name, val, addr ); */
    /* fprintf( stderr, "write register [0x%08x]%s <- %02x (byte %u)\n", addr, reg->name, val, addr ); */
    lcdc = val;

    enabled = ( ( lcdc & 0x80 ) != 0 );
    if ( enabled ) {
        ;; /* fprintf( stderr, "lcd enabled\n" ); */
    } else {
        if ( ly < 144 ) {
            fprintf( stderr, "warning: lcd disabled outside of vblank!\n" );
        } else {
            ;; /* fprintf( stderr, "lcd disabled\n" ); */
        }
        for ( y = 0; y < 144; y++ ) {
            for ( x = 0; x < 160; x++ ) {
                IO_DrawPixel( x, y, colors[0][0], colors[0][1], colors[0][2] );
            }
        }
        ly = 0;
        dotClock = dotDelay = dotDelayTotal = 0;
    }
}

static void Step( gbPpu_t *self ) {
    unsigned char shade = 0;
    unsigned short tileDataBase = 0x0000, bgMapBase= 0x0000, winMapBase = 0x0000;
    int i;

    lcdStat = lcdStat & 0xfc;

    if ( dotDelay > 0 ) {
        dotDelay--;
        return;
    }

    if ( dmaTransferActive ) {
        unsigned char val = self->cpu->bus->Read8( self->cpu->bus, dmaAddress, 0 );
        self->oam->Write8( self->oam, dmaAddress & 0x00ff, val, 0 );
        dmaAddress++;
        if ( ( dmaAddress & 0x00ff ) > 0x9f ) {
            dmaTransferActive = 0;
            if ( ( lcdStat & 0x20 ) != 0 ) {
                self->cpu->Interrupt( self->cpu, 1 );
            }
        }
    }

    if ( ( dotClock < 160 ) && ( ly < 144 ) ) {
        unsigned short bgLine, bgRow;
        if ( ( lcdc & 0x10 ) == 0 ) {
            tileDataBase += 0x800;
        }
        /*  */
        /*  render background */
        /*  */
        if ( ( lcdc & 0x01 ) != 0 ) {
            unsigned char tileLine, tileRow, tileNum, pixLine, pixRow, upperByte, lowerByte, palIndex;
            unsigned short tileOffset, tileByteIndex;
            if ( ( lcdc & 0x08 ) != 0 ) {
                bgMapBase = 0x400;
            }
            bgLine = scy + ly;
            bgRow = scx + dotClock;
            tileLine = bgLine / 8;
            tileRow = bgRow / 8;
            tileOffset = ( (unsigned short)tileLine * (unsigned short)32 ) + (unsigned short)tileRow;
            tileNum = self->bgRam->Read8( self->bgRam, bgMapBase + tileOffset, 0 ); 

            if ( ( lcdc & 0x10 ) == 0 ) {
                char tileFix = (char)tileNum;
                tileFix = tileFix + 128;
                tileNum = tileFix;
            }

            pixLine = bgLine % 8;
            pixRow = bgRow % 8;
            tileByteIndex = pixLine * 2;

            upperByte = self->cRam->Read8( self->cRam, tileDataBase + ( tileNum * 16 ) + tileByteIndex, 0 );
            lowerByte = self->cRam->Read8( self->cRam, tileDataBase + ( tileNum * 16 ) + tileByteIndex + 1, 0 );
            palIndex = ( ( upperByte >> ( 7 - pixRow ) ) & 0x1 );
            palIndex |= ( ( lowerByte >> ( 7 - pixRow ) ) & 0x1 ) << 1;
            shade = ( bgPal >> ( 2 * palIndex ) ) & 0x03;
        }
        /*  */
        /*  render window */
        /*  */
        bgLine = ly - wy - 1;
        bgRow = dotClock - ( wx - 8 );
        if ( ( ( lcdc & 0x20 ) != 0 ) && ( wx >= 0 ) && ( wy >= 0 ) && ( wx < 167 ) & ( wy < 144 ) &&
                            ( bgLine >= 0 ) && ( bgLine < 144 ) && ( bgRow >= 0 ) && ( bgRow < 161 ) ) {
            unsigned char tileLine, tileRow, tileNum, pixLine, pixRow, upperByte, lowerByte, palIndex;
            unsigned short tileOffset, tileByteIndex;
            if ( ( lcdc & 0x40 ) != 0 ) {
                winMapBase = 0x400;
            }
            tileLine = bgLine / 8;
            tileRow = bgRow / 8;
            tileOffset = ( (unsigned short)tileLine * (unsigned short)32 ) + (unsigned short)tileRow;
            tileNum = self->bgRam->Read8( self->bgRam, winMapBase + tileOffset, 0 ); 

            if ( ( lcdc & 0x10 ) == 0 ) {
                char tileFix = (char)tileNum;
                tileFix = tileFix + 128;
                tileNum = tileFix;
            }

            pixLine = bgLine % 8;
            pixRow = bgRow % 8;
            tileByteIndex = pixLine * 2;

            upperByte = self->cRam->Read8( self->cRam, tileDataBase + ( tileNum * 16 ) + tileByteIndex, 0 );
            lowerByte = self->cRam->Read8( self->cRam, tileDataBase + ( tileNum * 16 ) + tileByteIndex + 1, 0 );
            palIndex = ( ( upperByte >> ( 7 - pixRow ) ) & 0x1 );
            palIndex |= ( ( lowerByte >> ( 7 - pixRow ) ) & 0x1 ) << 1;
            shade = ( bgPal >> ( 2 * palIndex ) ) & 0x03;
        }
        /*  */
        /*  render sprites */
        /*  */
        if ( ! dmaTransferActive ) {
            unsigned char numSprites = 0;
            unsigned char bestSprites[40];
            unsigned char bestX = 168, bestY = 160, bestSprite = 255, bestAttr = 0x00;
            int spriteNum;

            memset( bestSprites, 255, 40 );
            for ( spriteNum = 39; spriteNum >= 0; spriteNum-- ) {
                unsigned char spriteY, spriteX = self->oam->Read8( self->oam, ( spriteNum * 4 ) + 1, 0 );
                unsigned char spriteAttr = self->bgRam->Read8( self->oam, ( spriteNum* 4 ) + 3, 0 );
                if ( ! ( ( spriteX > 0 ) && ( spriteX < 168 ) && ( dotClock < spriteX ) && ( dotClock >= ( spriteX - 8 ) ) ) )
                    continue;
                spriteY = self->oam->Read8( self->oam, ( spriteNum * 4 ), 0 );
                if ( ! ( ( spriteY > 0 ) && ( spriteY < 160 ) && ( ly < spriteY  ) && ( ly >= ( spriteY - 16 ) ) ) )
                    continue;
                if ( spriteX > bestX )
                    continue;
                bestX = spriteX;
                bestY = spriteY;
                bestSprite = spriteNum;
                bestAttr = spriteAttr;
                bestSprites[numSprites++] = bestSprite;
            }
            tileDataBase = 0;
            for ( i = 0; i < 10; i++ ) {
                unsigned char spriteTile;
                unsigned char spritePixRow;
                unsigned char spritePixLine;
                unsigned char tileByteIndex;

                bestSprite = bestSprites[i];
                if ( bestSprite == 255 )
                    break;
                bestX = self->oam->Read8( self->oam, ( bestSprite * 4 ) + 1, 0 );
                bestY = self->oam->Read8( self->oam, ( bestSprite * 4 ), 0 );
                bestAttr = self->bgRam->Read8( self->oam, ( bestSprite* 4 ) + 3, 0 );
                spriteTile = self->bgRam->Read8( self->oam, ( bestSprite * 4 ) + 2, 0 );
                spritePixRow = bestX - dotClock - 1;
                spritePixLine = bestY - ly - 1;
                if ( ( bestAttr & 0x20 ) == 0 )
                    spritePixRow = 7 - spritePixRow;
                if ( ( bestAttr & 0x40 ) == 0 ) {
                    if ( ( lcdc & 0x04 ) == 0 ) {
                        spritePixLine = ( 7 - spritePixLine ) + 8;
                    } else {
                        spritePixLine = ( 15 - spritePixLine );
                    }
                }
                tileByteIndex = spritePixLine * 2;
                if ( ( ( lcdc & 0x04 ) != 0 ) || ( spritePixLine < 8 ) ) {
                    unsigned char upperByte = self->cRam->Read8( self->cRam, tileDataBase + ( spriteTile * 16 ) + tileByteIndex, 0 );
                    unsigned char lowerByte = self->cRam->Read8( self->cRam, tileDataBase + ( spriteTile * 16 ) + tileByteIndex + 1, 0 );
                    unsigned char palIndex = ( ( upperByte >> ( 7 - spritePixRow ) ) & 0x1 );
                    palIndex |= ( ( lowerByte >> ( 7 - spritePixRow ) ) & 0x1 ) << 1;
                    if ( palIndex != 0 ) {
                        if ( ( bestAttr & 0x10 ) == 0 )
                            shade = ( objPal0 >> ( 2 * palIndex ) ) & 0x03;
                        else
                            shade = ( objPal1 >> ( 2 * palIndex ) ) & 0x03;
                        break;
                    }
                }
            }   
        }

        if ( enabled )
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

gbPpu_t *GbPpu( busDevice_t *bus, sm83_t *cpu, busDevice_t *bgRam, busDevice_t *cRam, busDevice_t *oam ) {
    gbPpu_t *ppu = malloc( sizeof( gbPpu_t ) );
    ppu->Step = Step;
    ppu->cpu = cpu;
    ppu->bgRam = bgRam;
    ppu->cRam = cRam;
    ppu->oam = oam;
    IO_Init( 640 + 32, 576 + 32, 160, 144 );
    IO_SetBg( 0xf0, 0xf0, 0xd0 );

    enabled = 1;
    lcdc = lcdStat = 0;
    scy = scx = wy = wx = 0;
    bgPal = objPal0 = objPal1 = 0;

    GenericBusMapping( bus, "LCDC", 0xFF40, 0xFF40, GenericRegister( "LCDC", &lcdc, 1, NULL, ControlRegisterWrite ) );
    GenericBusMapping( bus, "LCDStat", 0xFF41, 0xFF41, GenericRegister( "LCDStat", &lcdStat, 1, NULL, NULL ) );
    GenericBusMapping( bus, "SCY", 0xFF42, 0xFF42, GenericRegister( "SCY", &scy, 1, NULL, NULL ) );
    GenericBusMapping( bus, "SCX", 0xFF43, 0xFF43, GenericRegister( "SCX", &scx, 1, NULL, NULL ) );
    GenericBusMapping( bus, "LY", 0xFF44, 0xFF44, GenericRegister( "LY", &ly, 1, NULL, NULL ) );
    GenericBusMapping( bus, "LYC", 0xFF45, 0xFF45, GenericRegister( "LYC", &lyc, 1, NULL, NULL ) );
    GenericBusMapping( bus, "OamDma", 0xFF46, 0xFF46, GenericRegister( "OamDma", NULL, 1, NULL, DmaRegisterWrite ) );
    GenericBusMapping( bus, "BGP", 0xFF47, 0xFF47, GenericRegister( "BGP", &bgPal, 1, NULL, NULL ) );
    GenericBusMapping( bus, "OBP0", 0xFF48, 0xFF48, GenericRegister( "OBP0", &objPal0, 1, NULL, NULL ) );
    GenericBusMapping( bus, "OBP1", 0xFF49, 0xFF49, GenericRegister( "OBP1", &objPal1, 1, NULL, NULL ) );
    GenericBusMapping( bus, "WY", 0xFF4A, 0xFF4A, GenericRegister( "WY", &wy, 1, NULL, NULL ) );
    GenericBusMapping( bus, "WX", 0xFF4B, 0xFF4B, GenericRegister( "WX", &wx, 1, NULL, NULL ) );

    return ppu;
}
