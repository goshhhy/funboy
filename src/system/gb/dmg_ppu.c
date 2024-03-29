/* rewritten, scanline-based ppu emulation */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "version.h"
#include "device.h"
#include "alarms.h"
#include "sm83.h"
#include "io.h"
#include "ppu.h"

/*  register values 
TODO: move these into GbPpu_t so we can have multiple instances
(e.g. on classic mac, multiple gameboys running)
*/

static unsigned char lcdc;
static unsigned char lcdStat;
static unsigned char scx, scy, wx, wy;
static unsigned char ly, lyc;
static unsigned char bgPal, objPal0, objPal1;
static int dmaTransferActive = 0;
static unsigned short dmaAddress;
static unsigned char windowLines;
 
static int enabled;

static unsigned char colors[4][3] = {
	{ 0xe7, 0xe7, 0xc7 },
	{ 0xc3, 0xc3, 0x9f },
	{ 0x9b, 0x9b, 0x87 },
	{ 0x38, 0x38, 0x28 },
};

static void RenderLine( gbPpu_t *self );

static void DmaAlarmCallback( void * data ) {
    gbPpu_t *self = data;
    ramInfo_t *oam = self->oam->data;

    if ( dmaTransferActive ) {
        unsigned char val = self->cpu->bus->Read8( self->cpu->bus, dmaAddress, 0 );
        self->oamBytes[dmaAddress & 0x00ff] = val;
        dmaAddress++;
        if ( (unsigned char)( dmaAddress & 0x00ff ) > (unsigned char)0x9f ) {
            if ( ( lcdStat & 0x20 ) != 0 ) {
                self->cpu->Interrupt( self->cpu, 1 );
            }
            oam->disabled = 0;
            dmaTransferActive = 0;
        } else {
            self->dmaAlarm.when = 4;
            self->alarmManager->AlarmChangedCallback(self->alarmManager->alarmChangedCallbackData);
        }
    } else {
        oam->disabled = 1;
        dmaTransferActive = 1;

        self->dmaAlarm.when = 4;
        self->alarmManager->AlarmChangedCallback(self->alarmManager->alarmChangedCallbackData);
    }
}

static void HsyncAlarmCallback( void * data ) {
    gbPpu_t *self = data;

    if ( ly < 144 ) {
        RenderLine( self );
        if ( ( lcdStat & 0x08 ) != 0 ) {
            self->cpu->Interrupt( self->cpu, 1 );
        }
        lcdStat &= 0xFC;
        lcdStat |= 0x02;
    }

    self->hsyncAlarm.when = 456;
    self->alarmManager->AlarmChangedCallback(self->alarmManager->alarmChangedCallbackData);
}

static void VsyncAlarmCallback( void * data ) {
    gbPpu_t *self = data;

    ly++;
    if ( ly > 153 ) {
        ly = 0;
    }
    if ( ly < 144 ) {
        lcdStat &= 0xFC;
    } 
    if ( ly == 144 ) {
        self->cpu->Interrupt( self->cpu, 0 );
        windowLines = 0;
        if ( ( lcdStat & 0x10 ) != 0 ) {
            self->cpu->Interrupt( self->cpu, 1 );
        }
        lcdStat &= 0xFC;
        lcdStat |= 0x01;
    }
    if ( ly == lyc )  {
        if ( ( lcdStat & 0x40 ) != 0 ) {
            self->cpu->Interrupt( self->cpu, 1 );
        }
        lcdStat = lcdStat | 0x04;
    } else {
        lcdStat = lcdStat & 0xfb;
    }

    self->vsyncAlarm.when = 456;
    self->alarmManager->AlarmChangedCallback(self->alarmManager->alarmChangedCallbackData);
}

static void DmaRegisterWrite( busDevice_t *dev, busAddress_t addr, unsigned char val, int final ) {
    regInfo_t * reg;
    gbPpu_t * self;

    if ( !dev || !dev->data )
        return;
    reg = dev->data;
    self = reg->data;

    dmaAddress = (unsigned short)val << 8;
    dmaTransferActive = 0;

    self->dmaAlarm.when = self->alarmManager->AlarmGetTimePassedCallback(self->alarmManager->alarmGetTimePassedCallbackData) + 4;
    self->alarmManager->AlarmChangedCallback(self->alarmManager->alarmChangedCallbackData);
}

static void ControlRegisterWrite( busDevice_t *dev, busAddress_t addr, unsigned char val, int final ) {
    regInfo_t *reg;
    gbPpu_t *self;
    int x, y;
    int set_enabled;

    if ( !dev || !dev->data )
        return;
    reg = dev->data;
    self = reg->data;

    if ( addr > reg->len ) {
        fprintf( stderr, "warning: ControlRegisterWrite: address out of bounds\n" );
        return;
    }

    lcdc = val;

    set_enabled = ( ( lcdc & 0x80 ) != 0 );
    if ( set_enabled ) {
        ;; /* printf( "lcd enabled\n" ); */
        if ( !enabled ) {
            self->hsyncAlarm.when = self->alarmManager->AlarmGetTimePassedCallback(self->alarmManager->alarmGetTimePassedCallbackData) + 160 + 1;
            self->vsyncAlarm.when = self->alarmManager->AlarmGetTimePassedCallback(self->alarmManager->alarmGetTimePassedCallbackData) + 456 + 1;
            self->alarmManager->AlarmChangedCallback(self->alarmManager->alarmChangedCallbackData);
        }
    } else {
        /* printf( "lcd disabled\n" ); */
        if ( enabled ) {
            if ( ly < 144 ) {
                fprintf( stderr, "warning: lcd disabled outside of vblank!\n" );
            }
            self->hsyncAlarm.when = -1;
            self->vsyncAlarm.when = -1;
            self->alarmManager->AlarmChangedCallback(self->alarmManager->alarmChangedCallbackData);
        }
        for ( y = 0; y < 144; y++ ) {
            for ( x = 0; x < 160; x++ ) {
                /*IO_DrawPixel24( x, y, colors[0][0], colors[0][1], colors[0][2] );*/
                IO_DrawPixel8( x, y, 0 );
            }
        }
        ly = 0;

    }
    enabled = set_enabled;
}

static unsigned char DmaRegisterRead( busDevice_t *dev, busAddress_t addr, int final ) {
    return dmaAddress >> 8;
}

static unsigned char ControlRegisterRead( busDevice_t *dev, busAddress_t addr, int final ) {
    return lcdc;
}

static void RenderBackground( gbPpu_t *self, unsigned char *bgPrio, unsigned char *shade, unsigned short tileDataBase  ) {

    unsigned short bgMapBase = 0;
    unsigned char tileLine, tileRow, pixLine, pixRow, upperByte, lowerByte, palIndex;
    unsigned char bgLine, bgRow;
    short tileNum;
    unsigned short tileOffset, tileByteIndex, limit = 160;
    int i, j;

    if ( ( ( lcdc & LCDC_BITS_WINDOW_ENABLE ) != 0 ) && ( wy < ly ) && ( wx < 160 ) ) {
        limit = wx;
    }

    if ( ( lcdc & LCDC_BITS_BG_ENABLE ) != 0 ) {
        if ( ( lcdc & LCDC_BITS_BG_MAP_SELECT ) != 0 ) {
            bgMapBase = 0x400;
        }

        bgLine = scy + ly;
        tileLine = bgLine / 8;
        bgRow = scx;

        for ( i = 0; i < limit; ) {
            tileRow = bgRow / 8; 
            tileOffset = ( tileLine * 32 ) + tileRow;

            if ( ( lcdc & LCDC_BITS_BG_WINDOW_DATA_SELECT ) == 0 ) {
                tileNum = (signed char)(self->bgRamBytes[ bgMapBase + tileOffset ]) + 128;
            } else {
                tileNum = (unsigned char)(self->bgRamBytes[ bgMapBase + tileOffset ]);
            }

            pixLine = bgLine % 8;
            pixRow = bgRow % 8;
            tileByteIndex = pixLine * 2;

            upperByte = self->cRamBytes[tileDataBase + ( tileNum * 16 ) + tileByteIndex ];
            lowerByte = self->cRamBytes[tileDataBase + ( tileNum * 16 ) + tileByteIndex + 1 ];
            for ( j = pixRow; (j < 8) && (i < limit); j++ ) {
                palIndex = ( ( upperByte >> ( 7 - pixRow ) ) & 0x1 );
                palIndex |= ( ( lowerByte >> ( 7 - pixRow ) ) & 0x1 ) << 1;
                shade[i] = ( bgPal >> ( 2 * palIndex ) ) & 0x03;
                bgPrio[i] = palIndex;
                bgRow++;
                pixRow++;
                i++;
            }
        }
    }
}

static void RenderWindow( gbPpu_t *self, unsigned char *bgPrio, unsigned char *shade, unsigned short tileDataBase  ) {
	unsigned short winMapBase;
    unsigned char tileLine, tileRow, tileNum, pixLine, pixRow, upperByte, lowerByte, palIndex;
    unsigned short tileOffset, tileByteIndex;
    int i, j;

    if ( ( ( lcdc & 0x20 ) != 0 ) && ( wx >= 0 ) && ( wy >= 0 ) && ( wx < 167 ) & ( wy < 144 ) &&
                        ( ly >= 0 ) && ( ly < 144 ) ) {
        if ( ( lcdc & 0x40 ) != 0 ) {
            winMapBase = 0x400;
        } else {
            winMapBase = 0x0;
        }
        
        if ( wy > ly )
            return;

        i = wx - 7;
        if ( i < 0 )
            i = 0;

        while ( i < 160 ) {

            if ( wx - 7 > i )
                continue;

            tileLine = windowLines / 8;
            tileRow = ( i - wx + 7 ) / 8;
            tileOffset = ( (unsigned short)tileLine * (unsigned short)32 ) + (unsigned short)tileRow;
            tileNum = self->bgRamBytes[ winMapBase + tileOffset ];

            if ( ( lcdc & LCDC_BITS_BG_WINDOW_DATA_SELECT ) == 0 ) {
                char tileFix = (signed char)tileNum;
                tileFix = tileFix + 128;
                tileNum = tileFix;
            }

            pixLine = windowLines % 8;
            pixRow = i % 8;
            tileByteIndex = pixLine * 2;

            upperByte = self->cRamBytes[ tileDataBase + ( tileNum * 16 ) + tileByteIndex ];
            lowerByte = self->cRamBytes[ tileDataBase + ( tileNum * 16 ) + tileByteIndex + 1 ];
            for ( j = pixRow; (j < 8) && (i < 160); j++ ) {
                palIndex = ( ( upperByte >> ( 7 - pixRow ) ) & 0x1 );
                palIndex |= ( ( lowerByte >> ( 7 - pixRow ) ) & 0x1 ) << 1;
                shade[i] = ( bgPal >> ( 2 * palIndex ) ) & 0x03;
                bgPrio[i] = palIndex;
                i++;
                pixRow++;
            }
        }
        windowLines++;
    }
}

static void RenderSprites( gbPpu_t *self, unsigned char *bgPrio, unsigned char *shade ) {						
    unsigned char *oamBytes;

	if ( ( ( lcdc & LCDC_BITS_OBJ_ENABLE ) != 0) && !dmaTransferActive ) {
        unsigned char sprite = 0;
        unsigned char priority[40], spriteCount = 0;
        int i, j;
        
        oamBytes = self->oamBytes;

        /* first, get the eligible sprites */
        for ( i = 0; i < 40; i++ ) {
            unsigned char spriteY = oamBytes[( i * 4 )];
            if ( ly < spriteY - 16 )
                continue;
            
            if ( ( lcdc & LCDC_BITS_OBJ_SIZE ) == 0 ) {
                if ( ly >= spriteY - 8 ) {
                    continue;
                }
            } else {
                if ( ly >= spriteY ) {
                    continue;
                }
            }
            priority[spriteCount++] = i;
        }

        if ( spriteCount == 0 )
            return;

        /* selection sort by x, since it's simple to implement and as i write this
            i am not awake enough to do it properly. 
            todo: replace with partial merge sort. 
            also todo: investigate caching results?
            */
        for ( i = 0; i < spriteCount; i++ ) {
            unsigned char xA = oamBytes[(priority[i] * 4) + 1];
            unsigned char minXSprite = i, minX = xA, minXSpriteVal;
            for ( j = i + 1; j < spriteCount; j++ ) {
                unsigned char xB = oamBytes[(priority[j] * 4) + 1];
                if ( xB < minX ) {
                    minX = xB;
                    minXSprite = j;
                }
            }
            if ( minXSprite != i ) {
                minXSpriteVal = priority[minXSprite];
                priority[minXSprite] = priority[i];
                priority[i] = minXSpriteVal;
            }
        }

        /* limit to 10 sprites per line */
        if ( spriteCount > 10 ) {
            spriteCount = 10;
        }

        /* render the actual sprites, in priority order, running from left to right */
        for ( i = spriteCount - 1; i >= 0; --i ) {
            unsigned char spriteTile, spriteX, spriteY, spriteAttr, spritePixRow, spritePixLine, tileByteIndex;

            sprite = priority[i];
            spriteX = oamBytes[ ( sprite * 4 ) + 1 ];
            spriteY = oamBytes[ sprite * 4 ];
            spriteAttr = oamBytes[( sprite * 4 ) + 3];
            spriteTile = oamBytes[( sprite * 4 ) + 2];
            spritePixLine = spriteY - ly - 1;

            if ( ( spriteAttr & OBJATTR_BITS_Y_FLIP ) == 0 ) {
                spritePixLine = ( 15 - spritePixLine );
            } else if ( ( lcdc & LCDC_BITS_OBJ_SIZE ) == 0 ) {
                spritePixLine -= 8;
            }

            if ( ( lcdc & LCDC_BITS_OBJ_SIZE ) != 0 ) {
                spriteTile &= 0xFE;
            }

            tileByteIndex = spritePixLine * 2;
            
            j = spriteX - 8;
            if ( j < 0 )
                j = 0;

            for ( ; ( j < spriteX ) && ( j < 160 ) ; j++ ) {
                unsigned char upperByte, lowerByte, palIndex;
                if ( ( spriteAttr & OBJATTR_BITS_X_FLIP ) == 0 ) {
                    spritePixRow = j - ( spriteX - 8 );
                } else {
                    spritePixRow = 7 - ( j - ( spriteX - 8 ) );
                }

                upperByte = self->cRamBytes[ ( spriteTile * 16 ) + tileByteIndex ];
                lowerByte = self->cRamBytes[ ( spriteTile * 16 ) + tileByteIndex + 1 ];
                palIndex = ( ( upperByte >> ( 7 - spritePixRow ) ) & 0x1 );
                palIndex |= ( ( lowerByte >> ( 7 - spritePixRow ) ) & 0x1 ) << 1;
                if ( palIndex != 0 ) {
                    unsigned char s;

                    if ( ( spriteAttr & OBJATTR_BITS_LAYER ) != 0 ) {
                        if ( bgPrio[j] > 0 ) {
                            continue;
                        }
                    }

                    if ( ( spriteAttr & OBJATTR_BITS_DMG_PALNUM ) == 0 )
                        s = ( objPal0 >> ( 2 * palIndex ) ) & 0x03;
                    else
                        s = ( objPal1 >> ( 2 * palIndex ) ) & 0x03;

                    shade[j] = s;
                }
            }
        }
    }
}

static void RenderLine( gbPpu_t *self ) {
    unsigned char bgPrio[160], shades[160];
    unsigned short tileDataBase = 0;
    int i;

    if ( ( lcdc & LCDC_BITS_BG_WINDOW_DATA_SELECT ) == 0 ) {
        tileDataBase += 0x800;
    }

    memset( &shades, 0, sizeof(shades) );
    memset( &bgPrio, 0, sizeof(bgPrio) );

    if ( enabled ) {
        RenderBackground( self, bgPrio, shades, tileDataBase );
        RenderWindow( self, bgPrio, shades, tileDataBase );
        RenderSprites( self, bgPrio, shades );
        for ( i = 0; i < 160; i++ ) {
            /*IO_DrawPixel24( i, ly, colors[shades[i]][0], colors[shades[i]][1], colors[shades[i]][2] );*/
            IO_DrawPixel8( i, ly, shades[i] );
        }
    }
}

static void Step( gbPpu_t *self ) {
    (void)self;
}

gbPpu_t *GbPpu( busDevice_t *bus, sm83_t *cpu, busDevice_t *bgRam, busDevice_t *cRam, busDevice_t *oam, alarmManager_t * alarmManager ) {
    int i;
    gbPpu_t *ppu = malloc( sizeof( gbPpu_t ) );
    ppu->Step = Step;
    ppu->cpu = cpu;
    ppu->bgRam = bgRam;
    ppu->cRam = cRam;
    ppu->oam = oam;

    ppu->alarmManager = alarmManager;

    ppu->dmaAlarm.name = "dmaAlarm";
    ppu->dmaAlarm.when = -1;
    ppu->dmaAlarm.Callback = DmaAlarmCallback;
    ppu->dmaAlarm.callbackData = ppu;

    ppu->hsyncAlarm.name = "hsyncAlarm";
    ppu->hsyncAlarm.when = 160;
    ppu->hsyncAlarm.Callback = HsyncAlarmCallback;
    ppu->hsyncAlarm.callbackData = ppu;

    ppu->vsyncAlarm.name = "vsyncAlarm";
    ppu->vsyncAlarm.when = 456;
    ppu->vsyncAlarm.Callback = VsyncAlarmCallback;
    ppu->vsyncAlarm.callbackData = ppu;

    AlarmAdd( alarmManager, &ppu->dmaAlarm );
    AlarmAdd( alarmManager, &ppu->hsyncAlarm );
    AlarmAdd( alarmManager, &ppu->vsyncAlarm );
    /* the bus abstraction is useful from the cpu perspective, but here we know exactly what we're accessing
    so it just slows us down. bypassing it here provides a massive speedup. without this, RenderSprites ends
    up taking upwards of 40% of the total execution time of the emulator */
    ppu->bgRamBytes = bgRam->DataPtr(bgRam);
    ppu->cRamBytes = cRam->DataPtr(cRam);
    ppu->oamBytes = oam->DataPtr(oam);

    IO_Init( 640, 576, 160, 144 );
    IO_SetBg( 0xf0, 0xf0, 0xd0 );

    for ( i = 0; i < 4; i++ ) {
        IO_SetPaletteColor( i, colors[i][0], colors[i][1], colors[i][2] );
    }

    enabled = 1;
    lcdc = lcdStat = 0;
    scy = scx = wy = wx = 0;
    bgPal = objPal0 = objPal1 = 0;

    GenericBusMapping( bus, "LCDC", 0xFF40, 0xFF40, GenericRegister( "LCDC", ppu, 1, ControlRegisterRead, ControlRegisterWrite ) );
    GenericBusMapping( bus, "LCDStat", 0xFF41, 0xFF41, GenericRegister( "LCDStat", &lcdStat, 1, NULL, NULL ) );
    GenericBusMapping( bus, "SCY", 0xFF42, 0xFF42, GenericRegister( "SCY", &scy, 1, NULL, NULL ) );
    GenericBusMapping( bus, "SCX", 0xFF43, 0xFF43, GenericRegister( "SCX", &scx, 1, NULL, NULL ) );
    GenericBusMapping( bus, "LY", 0xFF44, 0xFF44, GenericRegister( "LY", &ly, 1, NULL, GenericRegisterReadOnly ) );
    GenericBusMapping( bus, "LYC", 0xFF45, 0xFF45, GenericRegister( "LYC", &lyc, 1, NULL, NULL ) );
    GenericBusMapping( bus, "OamDma", 0xFF46, 0xFF46, GenericRegister( "OamDma", ppu, 1, DmaRegisterRead, DmaRegisterWrite ) );
    GenericBusMapping( bus, "BGP", 0xFF47, 0xFF47, GenericRegister( "BGP", &bgPal, 1, NULL, NULL ) );
    GenericBusMapping( bus, "OBP0", 0xFF48, 0xFF48, GenericRegister( "OBP0", &objPal0, 1, NULL, NULL ) );
    GenericBusMapping( bus, "OBP1", 0xFF49, 0xFF49, GenericRegister( "OBP1", &objPal1, 1, NULL, NULL ) );
    GenericBusMapping( bus, "WY", 0xFF4A, 0xFF4A, GenericRegister( "WY", &wy, 1, NULL, NULL ) );
    GenericBusMapping( bus, "WX", 0xFF4B, 0xFF4B, GenericRegister( "WX", &wx, 1, NULL, NULL ) );

    return ppu;
}
