#include <stdbool.h>

#include <SDL2/SDL.h>



#include "io.h"

SDL_Window* window = NULL;
SDL_Surface* windowSurf = NULL;
SDL_Surface* renderSurf = NULL;
SDL_Surface* pixelSurf = NULL;

int renderScale = 4;
int useScanLines = 0;
int useDotsFilter = 1;

uint8_t bgRed = 0, bgGreen = 0, bgBlue = 0;

int renderWidth, renderHeight;

int* screen = NULL;

void IO_SetRenderRes( int x, int y ) {
	renderWidth = x;
	renderHeight = y;

	if ( screen )
		free( screen );

	screen = malloc( ( x * y * sizeof( int ) * 3 ) * 2 );

	if ( screen == NULL ) {
		fprintf( stderr, "failedS to alloc screen memory\n" );
		exit( 1 ); 
	}
}

void IO_SetBg( uint8_t r, uint8_t g, uint8_t b ) {
	bgRed = r;
	bgGreen = g;
	bgBlue = b;
}

int IO_Init( int wWidth, int wHeight, int rWidth, int rHeight ) {
	window = SDL_CreateWindow( "kutaragi!", -1, -1, wWidth, wHeight, SDL_WINDOW_RESIZABLE );
	if ( !window ) {
		printf( "failed to create window\n" );
		return 1;
	}

	IO_SetRenderRes( rWidth, rHeight );

	windowSurf = SDL_GetWindowSurface( window );
	renderSurf = SDL_CreateRGBSurface( 0, rWidth * renderScale, rHeight * renderScale, 32, 0xFF, 0xFF << 8, 0xFF << 16, 0xFF << 24 );
	pixelSurf = SDL_CreateRGBSurface( 0, 1 * renderScale, 1 * renderScale, 32, 0xFF, 0xFF << 8, 0xFF << 16, 0xFF << 24 );

	SDL_FillRect( windowSurf, NULL, SDL_MapRGB( windowSurf->format, 0x00, 0x00, 0x00 ) );
	SDL_FillRect( renderSurf, NULL, SDL_MapRGB( renderSurf->format, 0x00, 0x00, 0x22 ) );
	SDL_FillRect( pixelSurf, NULL, SDL_MapRGB( pixelSurf->format, 0x00, 0x00, 0x00 ) );
	SDL_UpdateWindowSurface( window );

	return 0;
}

void IO_DrawPixel( int x, int y, uint8_t r, uint8_t g, uint8_t b ) {
	screen[(renderWidth * y * 3) + (x * 3) + 0] = r;
	screen[(renderWidth * y * 3) + (x * 3) + 1] = g;
	screen[(renderWidth * y * 3) + (x * 3) + 2] = b;
}

void IO_ScreenCopy( void ) {
	int x, y, r, g, b;

	for( y = 0; y < renderHeight; y++ ) {
		for ( x = 0; x < renderWidth; x++ ) {
			r = screen[(renderWidth * y * 3) + (x * 3) + 0];
			g = screen[(renderWidth * y * 3) + (x * 3) + 1];
			b = screen[(renderWidth * y * 3) + (x * 3) + 2];

			SDL_Rect dim = { 0, 0, 0, 0 };
			
			dim.x = x * renderScale;
			dim.y = y * renderScale;
			
			if ( useDotsFilter && ( renderScale > 2 ) ) {
				uint8_t averaged_r = ( (uint16_t)r + (uint16_t)r + (uint16_t)bgRed ) / 3;
				uint8_t averaged_g = ( (uint16_t)g + (uint16_t)g + (uint16_t)bgGreen ) / 3;
				uint8_t averaged_b = ( (uint16_t)b + (uint16_t)b + (uint16_t)bgBlue ) / 3;

				SDL_Rect dim2 = { 0, 0, 1 * renderScale - 1, 1 * renderScale};
				dim2.x = x * renderScale + (renderScale - 1);
				dim2.y = y * renderScale;
				SDL_Rect dim3 = { 0, 0, 1 * renderScale, 1 * renderScale - 1 };
				dim3.x = x * renderScale;
				dim3.y = y * renderScale + (renderScale - 1);
				SDL_FillRect( pixelSurf, NULL, SDL_MapRGB( pixelSurf->format, r, g, b ) );
				SDL_BlitSurface( pixelSurf, NULL, renderSurf, &dim );
				SDL_FillRect( pixelSurf, NULL, SDL_MapRGB( pixelSurf->format, averaged_r, averaged_g, averaged_b ) );
				SDL_BlitSurface( pixelSurf, NULL, renderSurf, &dim2 );
				SDL_FillRect( pixelSurf, NULL, SDL_MapRGB( pixelSurf->format, averaged_r, averaged_g, averaged_b ) );
				SDL_BlitSurface( pixelSurf, NULL, renderSurf, &dim3 );
			} else if ( useScanLines && ( renderScale > 1 ) ) {
				SDL_Rect dim2 = { 0, 0, 1 * renderScale, 1 * renderScale / 2 };
				dim2.x = x * renderScale;
				dim2.y = y * renderScale + (renderScale / 2);
				SDL_FillRect( pixelSurf, NULL, SDL_MapRGB( pixelSurf->format, r, g, b ) );
				SDL_BlitSurface( pixelSurf, NULL, renderSurf, &dim );
				SDL_FillRect( pixelSurf, NULL, SDL_MapRGB( pixelSurf->format, r * 0.85, g * 0.85, b * 0.85 ) );
				SDL_BlitSurface( pixelSurf, NULL, renderSurf, &dim2 );
			} else {
				SDL_FillRect( pixelSurf, NULL, SDL_MapRGB( pixelSurf->format, r, g, b ) );
				SDL_BlitSurface( pixelSurf, NULL, renderSurf, &dim );	
			}
		}
	}
}

bool IO_Update( void ) {
	SDL_Event e;
	static int updates = 0;
	bool r = true;

	printf( "begin video update %i\n", updates++ );
	IO_ScreenCopy();
	
	if ( SDL_BlitScaled( renderSurf, NULL, windowSurf, NULL ) )	
		fprintf( stderr, "update: scale failed: %s", SDL_GetError() );

	SDL_UpdateWindowSurface( window );
	while ( SDL_PollEvent( &e ) != 0 ) {
		switch( e.type ) {
			case SDL_QUIT:
				r = false;
				break;
			case SDL_WINDOWEVENT_RESIZED:
				windowSurf = SDL_GetWindowSurface( window );
				break;
			default:
				break;
		}
	}
	return r;
}