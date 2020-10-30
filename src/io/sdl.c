#include <SDL2/SDL.h>

#include "io.h"

SDL_Window* window = NULL;
SDL_Surface* windowSurf = NULL;
SDL_Surface* renderSurf = NULL;
SDL_Surface* pixelSurf = NULL;

static int borderSize = 4;
int renderScale = 4;
int useScanLines = 0;
int useDotsFilter = 1;

const char* emuName = "funboy!";

uint8_t bgRed = 0, bgGreen = 0, bgBlue = 0;

int renderWidth, renderHeight;

uint8_t* screen = NULL;

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
	bgRed = r;
	bgGreen = g;
	bgBlue = b;
}

int IO_Init( int wWidth, int wHeight, int rWidth, int rHeight ) {
	window = SDL_CreateWindow( "kutaragi!", -1, -1, wWidth, wHeight, 0 );
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

void IO_DrawPixel( int x, int y, uint8_t r, uint8_t g, uint8_t b ) {
	screen[(renderWidth * y * 3) + (x * 3) + 0] = r;
	screen[(renderWidth * y * 3) + (x * 3) + 1] = g;
	screen[(renderWidth * y * 3) + (x * 3) + 2] = b;
}

void IO_ScreenCopy( void ) {
	int x, y, r, g, b;

	SDL_FillRect( renderSurf, NULL, SDL_MapRGB( renderSurf->format, bgRed, bgGreen, bgBlue ) );

	for( y = 0; y < renderHeight; y++ ) {
		for ( x = 0; x < renderWidth; x++ ) {
			SDL_Rect dim, dim2, dim3;

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

				dim2.x = ( x + borderSize ) * renderScale + (renderScale - 1);
				dim2.y = ( y + borderSize ) * renderScale;
				dim2.w = 1 * renderScale - 1;
				dim2.h = 1 * renderScale;
				
				dim3.x = ( x + borderSize ) * renderScale;
				dim3.y = ( y + borderSize ) * renderScale + (renderScale - 1);
				dim3.w = 1 * renderScale;
				dim3.h = 1 * renderScale - 1;

				SDL_FillRect( renderSurf, &dim, SDL_MapRGB( renderSurf->format, r, g, b ) );
				SDL_FillRect( renderSurf, &dim2, SDL_MapRGB( pixelSurf->format, averaged_r, averaged_g, averaged_b ) );
				SDL_FillRect( renderSurf, &dim3, SDL_MapRGB( pixelSurf->format, averaged_r, averaged_g, averaged_b ) );
			} else if ( useScanLines && ( renderScale > 1 ) ) {
				dim.h = dim.h / 2;
				dim2.x = x * renderScale;
				dim2.y = y * renderScale + (renderScale / 2);
				dim2.w = 1 * renderScale;
				dim2.h = 1 * renderScale / 2;
				SDL_FillRect( pixelSurf, &dim, SDL_MapRGB( pixelSurf->format, r, g, b ) );
				SDL_FillRect( pixelSurf, &dim2, SDL_MapRGB( pixelSurf->format, r * 0.85, g * 0.85, b * 0.85 ) );
			} else {
				SDL_FillRect( pixelSurf, &dim, SDL_MapRGB( pixelSurf->format, r, g, b ) );
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

int IO_Update( void ) {
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
	return r;
}