#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "../../device/device.h"
#include "../../io/io.h"

uint8_t colors[16][3] = {
	{ 0x00, 0x00, 0x00 },
	{ 0xff, 0xff, 0xff },
	{ 0x88, 0x00, 0x00 },
	{ 0x22, 0xdd, 0xdd },
	{ 0xaa, 0x00, 0xbb },
	{ 0x00, 0xaa, 0x00 },
	{ 0x33, 0x33, 0x88 },
	{ 0xaa, 0xaa, 0x55 },
	{ 0x88, 0x55, 0x33 },
	{ 0xaa, 0x77, 0x55 },
	{ 0xaa, 0x55, 0x55 },
	{ 0xaa, 0xee, 0xee },
	{ 0xee, 0xaa, 0xdd },
	{ 0x77, 0xcc, 0x77 },
	{ 0x55, 0x55, 0xaa },
	{ 0xdd, 0xdd, 0x88 },
};

uint8_t cr[0x10];
uint8_t colorRam[512];

void Vic_SetCr( uint8_t which, uint8_t val ) {
	cr[which & 0xf] = val;
}

uint8_t Vic_GetCr( uint8_t which ) {
	return cr[which & 0xf];
}

void Vic_SetColorRam( uint16_t offset, uint8_t val ) {
	colorRam[offset] = val & 0x0f;
}

uint8_t Vic_GetColorRam( uint16_t offset ) {
	return colorRam[offset];
}

void Vic_Init( void ) {
	int i;
    IO_Init( 640, 270 * 2, 208, 270 );

	for ( i = 0; i < 8; i++ )
		IO_DrawPixel( i, 0, colors[i][0], colors[i][1], colors[i][2] );

	for ( i = 8; i < 16; i++ )
		IO_DrawPixel( i - 8, 1, colors[i][0], colors[i][1], colors[i][2] );
}

int Vic_Step( busDevice_t *bus ) {
	static int rasterPixel = 0;
	static long long int frame;
	int i, r = 0;
	int xOrigin = ( ( cr[0] & 0x7f ) * 4 ) , yOrigin = ( cr[1] * 2 ) - 14;
	int columns = cr[2] & 0x7f, rows = ( cr[3] & 0x7e ) >> 1;
	int rasterLine = ( ( ( cr[3] & 0x80 ) << 1 ) + cr[4] ) - 27;
	int borderColor = cr[0xf] & 0x07, bgColor = ( cr[0xf] & 0xf0 ) >> 4;
	int charColor = 6;
	int charRow, charCol, charX, charY;
	uint8_t c, cSub;
    uint16_t where;
	uint16_t screenRamBase = ( ( ( cr[5] & 0x70 ) << 6 ) + ( ( cr[2] & 0x80 ) << 2 ) );
	uint16_t characterRomBase = ( ( cr[5] & 0x08 ) << 10 ) + 0x8000; /* this is WRONG but works for now */
	uint16_t characterLoc;

	if ( rasterLine >= 0 ) { /* if not in vblank */
		for ( i = rasterPixel; i < rasterPixel + 4; i++ ) {
			IO_DrawPixel( i, rasterLine, colors[borderColor][0], colors[borderColor][1], colors[borderColor][2] );
			if ( ( rasterLine > yOrigin ) && ( rasterLine < ( yOrigin + ( rows * 8 ) ) ) ) {
				if ( ( i > xOrigin ) && ( i < ( xOrigin + ( columns * 8 ) ) ) ) {
					IO_DrawPixel( i, rasterLine, colors[bgColor][0], colors[bgColor][1], colors[bgColor][2] );
					
					/* calculate the current text row, and column */
					charRow = ( rasterLine - yOrigin ) / 8;
					charCol = ( i - xOrigin ) / 8;
					charX = ( ( i - xOrigin ) % 8 ) - 1;
					charY = ( ( rasterLine - yOrigin ) % 8 ) - 1;

					/* fetch that character cell from ram */
                    where = screenRamBase + ( charRow * columns ) + charCol;
					c = bus->Read8( bus, where, false );

					/* find the correct data in our character rom */
					characterLoc = c * 8;
					cSub = bus->Read8( bus, characterRomBase + characterLoc + charY, false );
					if ( ( ( cSub >> ( 7 - charX ) ) & 0x1 ) == 1 ) {
						IO_DrawPixel( i, rasterLine, colors[charColor][0], colors[charColor][1], colors[charColor][2] );
					} 
				}
			}
		}
	}
	rasterPixel += 4;
	if ( rasterPixel > 65 * 4 ) {
		rasterPixel = 0;
		rasterLine++;
		if ( rasterLine > 311 - 27 ) {
			rasterLine = -27;
			frame++;
			r = 1;
		}
		rasterLine += 27;
		cr[4] = rasterLine & 0x0FF;
		cr[3] = ( cr[3] & 0x7F ) | ( ( rasterLine >> 1 ) & 0x80 );
	}
	return r;
}

static uint8_t VicVideoRead( busDevice_t *dev, uint32_t addr, bool final ) {
    if ( !dev )
        return 0;
    if ( addr > 0x10 ) {
        fprintf( stderr, "warning: VicVideoRead: address out of bounds\n" );
        return 0;
    }
    return Vic_GetCr( addr );
}


static void VicVideoWrite( busDevice_t *dev, uint32_t addr, uint8_t val, bool final ) {
    if ( !dev )
        return;

    if ( addr > 0x10 ) {
        fprintf( stderr, "warning: VicVideoWrite: address out of bounds\n" );
        return;
    }
    Vic_SetCr( addr, val );
}

busDevice_t *VicVideo( void ) {
    busDevice_t *dev;

    if ( !( dev = malloc( sizeof( busDevice_t ) ) ) ) {
        fprintf( stderr, "couldn't allocate ram for video busDevice\n" );
        return NULL;
    }
    Vic_Init();
    dev->data = NULL;
    dev->Read8 = VicVideoRead;
    dev->Write8 = VicVideoWrite;
    printf( "created VIC video device\n" );
    return dev;
}
