#include <SDL.h>
#include <time.h>

#include "device.h"
#include "sm83.h"
#include "io.h"
#include "gb.h"

SDL_Window* window = NULL;
SDL_Surface* windowSurf = NULL;
SDL_Surface* renderSurf = NULL;
SDL_Surface* pixelSurf = NULL;

static int borderSize = 4;
int renderScale = 4;
int useScanLines = 0;
int useDotsFilter = 1;

int benchmark = 0;
int benchmark_limit = 0;
clock_t benchmark_time;

const char* emuName = "funboy!";

uint8_t bgRed = 0, bgGreen = 0, bgBlue = 0;

int renderWidth, renderHeight;

uint8_t* screen = NULL;

unsigned char palette[768];

void (*KeyPressCallback)( int key ) = NULL;
void (*KeyReleaseCallback)( int key ) = NULL;

void IO_SetRenderRes( int x, int y ) {
	renderWidth = x;
	renderHeight = y;

	if ( screen )
		free( screen );

	screen = malloc( ( ( x + ( 2 * borderSize ) ) * ( y + ( 2 * borderSize ) )* sizeof( int ) * 3 ) * 2 );

	if ( screen == NULL ) {
		fprintf( stderr, "failed to alloc screen memory\n" );
		exit( 1 ); 
	}
}

void IO_SetBg( uint8_t r, uint8_t g, uint8_t b ) {
	IO_SetPaletteColor( 255, r, g, b );
	bgRed = r;
	bgGreen = g;
	bgBlue = b;
}

int IO_Init( int wWidth, int wHeight, int rWidth, int rHeight ) {
	window = SDL_CreateWindow( "kutaragi!", -1, -1, ( rWidth + ( 2 * borderSize ) ) * renderScale, ( rHeight + ( 2 * borderSize ) ) * renderScale, 0 );
	if ( !window ) {
		printf( "failed to create window\n" );
		return 1;
	}

	IO_SetRenderRes( rWidth, rHeight );

	windowSurf = SDL_GetWindowSurface( window );
	renderSurf = SDL_CreateRGBSurface( 0, ( rWidth + ( 2 * borderSize ) ) * renderScale, ( rHeight + ( 2 * borderSize ) ) * renderScale, 32, 0xFF, 0xFF << 8, 0xFF << 16, 0xFF << 24 );
	pixelSurf = SDL_CreateRGBSurface( 0, 1 * renderScale, 1 * renderScale, 32, 0xFF, 0xFF << 8, 0xFF << 16, 0xFF << 24 );

	SDL_FillRect( windowSurf, NULL, SDL_MapRGB( windowSurf->format, 0x00, 0x00, 0x00 ) );
	SDL_FillRect( renderSurf, NULL, SDL_MapRGB( renderSurf->format, 0x00, 0x00, 0x22 ) );
	SDL_FillRect( pixelSurf, NULL, SDL_MapRGB( pixelSurf->format, 0x00, 0x00, 0x00 ) );
	SDL_UpdateWindowSurface( window );

	benchmark_time = clock();

	return 0;
}

void IO_SetEmuName( const char* name ) {
	emuName = name;
} 

void IO_SetTitle( const char* title ) {
	char final[256] = "";
	strcat( final, emuName );
	strcat( final, " - " );
	strncat( final, title, 16 );
	SDL_SetWindowTitle( window, final );
}

void IO_DrawPixel8( int x, int y, unsigned char index ) {
	screen[(renderWidth * y * 3) + (x * 3) + 0] = palette[index * 3];
	screen[(renderWidth * y * 3) + (x * 3) + 1] = palette[index * 3 + 1];
	screen[(renderWidth * y * 3) + (x * 3) + 2] = palette[index * 3 + 2];
}

void IO_DrawPixel24( int x, int y, uint8_t r, uint8_t g, uint8_t b ) {
	screen[(renderWidth * y * 3) + (x * 3) + 0] = r;
	screen[(renderWidth * y * 3) + (x * 3) + 1] = g;
	screen[(renderWidth * y * 3) + (x * 3) + 2] = b;
}

void IO_ScreenCopy( void ) {
	int x, y, r, g, b;

	SDL_FillRect( renderSurf, NULL, SDL_MapRGB( renderSurf->format, bgRed, bgGreen, bgBlue ) );

	for( y = 0; y < renderHeight; y++ ) {
		for ( x = 0; x < renderWidth; x++ ) {
			SDL_Rect dim, dim2;

			r = screen[(renderWidth * y * 3) + (x * 3) + 0];
			g = screen[(renderWidth * y * 3) + (x * 3) + 1];
			b = screen[(renderWidth * y * 3) + (x * 3) + 2];
			
			dim.x = ( x + borderSize ) * renderScale;
			dim.y = ( y + borderSize ) * renderScale;
			dim.w = renderScale;
			dim.h = renderScale;
			
			if ( useDotsFilter && ( renderScale > 2 ) ) {
				uint8_t averaged_r = ( (uint16_t)r + (uint16_t)r + (uint16_t)bgRed ) / 3;
				uint8_t averaged_g = ( (uint16_t)g + (uint16_t)g + (uint16_t)bgGreen ) / 3;
				uint8_t averaged_b = ( (uint16_t)b + (uint16_t)b + (uint16_t)bgBlue ) / 3;

				dim.w -= 1;
				dim.h -= 1;

				dim2.x = dim.x;
				dim2.y = dim.y;
				dim2.w = dim.w + 1;
				dim2.h = dim.h + 1;
				
				SDL_FillRect( renderSurf, &dim2, SDL_MapRGB( pixelSurf->format, averaged_r, averaged_g, averaged_b ) );
				SDL_FillRect( renderSurf, &dim, SDL_MapRGB( renderSurf->format, r, g, b ) );
			} else if ( useScanLines && ( renderScale > 1 ) ) {
				dim.h = dim.h / 2;
				dim2.x = x * renderScale;
				dim2.y = y * renderScale + (renderScale / 2);
				dim2.w = 1 * renderScale;
				dim2.h = 1 * renderScale / 2;
				SDL_FillRect( pixelSurf, &dim, SDL_MapRGB( pixelSurf->format, r, g, b ) );
				SDL_FillRect( pixelSurf, &dim2, SDL_MapRGB( pixelSurf->format, r * 0.85, g * 0.85, b * 0.85 ) );
			} else {
				SDL_FillRect( renderSurf, &dim, SDL_MapRGB( pixelSurf->format, r, g, b ) );
			}
		}
	}
}

void IO_SetKeyPressCallback( void (*Callback)( int key ) ) {
	KeyPressCallback = Callback;
}

void IO_SetKeyReleaseCallback( void (*Callback)( int key ) ) {
	KeyReleaseCallback = Callback;	
}

int IO_Update( sm83_t * cpu ) {
	SDL_Event e;
	int r = 1;

	/*printf( "begin video update %i\n", updates++ );*/
	IO_ScreenCopy();
	
	if ( SDL_BlitScaled( renderSurf, NULL, windowSurf, NULL ) )	
		fprintf( stderr, "update: scale failed: %s", SDL_GetError() );

	SDL_UpdateWindowSurface( window );
	while ( SDL_PollEvent( &e ) != 0 ) {
		switch( e.type ) {
			case SDL_QUIT:
				r = 0;
				break;
			case SDL_WINDOWEVENT_RESIZED:
				windowSurf = SDL_GetWindowSurface( window );
				break;
			case SDL_KEYDOWN:
				switch( e.key.keysym.scancode ) {
					case 47:
						Sm83_SetInterpreterMode( cpu, INTERPRETER_MODE_STANDARD );
						break;
					case 48:
						Sm83_SetInterpreterMode( cpu, INTERPRETER_MODE_CACHED );
						break;
					default:
						break;
				}
				if ( KeyPressCallback )
					KeyPressCallback( e.key.keysym.scancode );
				break;
			case SDL_KEYUP:
				if ( KeyReleaseCallback )
					KeyReleaseCallback( e.key.keysym.scancode );
				break;
			default:
				break;
		}
	}

	if ( benchmark_limit > 0 ) {
		if ( ++benchmark == benchmark_limit ) {
			clock_t end = clock();
			r = 0;

			printf( "%i frames in %f cpu secs\n", benchmark, ((double)(end - benchmark_time)) / CLOCKS_PER_SEC );
		}
	}

	return r;
}

void IO_SetPaletteColor( int index, unsigned char r, unsigned char g, unsigned char b ) {
	palette[index * 3] = r;
	palette[index * 3 + 1] = g;
	palette[index * 3 + 2] = b;
	return;
}

int printusage( void ) {
	printf("usage: funboy [filename] <benchmark_frames>\n");
	return 1;
}

int main( int argc, char **argv ) {

	if ( argc < 2 ) {
		return printusage();
	}
	
	if ( argc == 3 ) {
		benchmark_limit = atoi( argv[2] );
	}

	gb_main(argv[1]);
}