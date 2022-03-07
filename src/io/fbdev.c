#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <errno.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>a
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>

#include "device.h"
#include "sm83.h"
#include "io.h"
#include "gb.h"

int renderScale = 4;
int useScanLines = 0;
int useDotsFilter = 1;

int benchmark = 0;
int benchmark_limit = 0;
clock_t benchmark_time;

const char* emuName = "funboy!";

int renderWidth, renderHeight;
int xAdjust, yAdjust;

char * fbdev_path = "/dev/fb0";
struct fb_cmap cmap;
int fbdev;
volatile int stop = 0;
int linewidth = 0;

uint8_t* screen = NULL;
size_t screen_size = 0;

void (*KeyPressCallback)( int key ) = NULL;
void (*KeyReleaseCallback)( int key ) = NULL;

void SigHandler( int v ) {
	stop = 1;
}

void IO_SetRenderRes( int x, int y ) {
	memset( screen, 0, screen_size );	

	renderWidth = x;
	renderHeight = y;

	xAdjust = 160 - (renderWidth / 2);
	yAdjust = 120 - (renderHeight / 2);
}

void IO_SetBg( uint8_t r, uint8_t g, uint8_t b ) {
	IO_SetPaletteColor( 255, r, g, b );
}

static void cleanup( void ) {
	memset( screen, 0, screen_size );
	close(fbdev);
}

void PrintScreenInfo( char * preamble, struct fb_var_screeninfo vinfo ) {
	printf("fbdev: %s: %ix%i %ibpp %s\n",
		preamble,
		vinfo.xres, vinfo.yres,
		vinfo.bits_per_pixel,
		(vinfo.grayscale == 1) ? "grayscale" : "color");
}

int IO_Init( int wWidth, int wHeight, int rWidth, int rHeight ) {
	struct fb_fix_screeninfo finfo;
	struct fb_var_screeninfo vinfo;

	(void)wWidth, (void)wHeight;

	signal(SIGINT, SigHandler);

	fbdev = open( fbdev_path, O_RDWR );	
	if ( fbdev < 0 ) {
		printf( "failed to open device %s\n", fbdev_path );
		return 1;
	}
	atexit(cleanup);

	ioctl( fbdev, FBIOGET_FSCREENINFO, &finfo );
	linewidth = finfo.line_length;

	ioctl( fbdev, FBIOGET_VSCREENINFO, &vinfo );

	PrintScreenInfo( "before", vinfo );

	vinfo.bits_per_pixel = 8;
	vinfo.grayscale = 0;
	vinfo.xres = vinfo.xres_virtual = 640;
	vinfo.yres = vinfo.yres_virtual = 240;

	vinfo.pixclock = 42109;
	vinfo.left_margin = 80;
	vinfo.right_margin = 16;
	vinfo.upper_margin = 13;
	vinfo.lower_margin = 1;
	vinfo.hsync_len = 64;
	vinfo.vsync_len = 3;	
	vinfo.sync = FB_SYNC_VERT_HIGH_ACT;
	vinfo.vmode = FB_VMODE_DOUBLE;
	
	ioctl( fbdev, FBIOPUT_VSCREENINFO, &vinfo );
	ioctl( fbdev, FBIOGET_VSCREENINFO, &vinfo );
	
	PrintScreenInfo( "after", vinfo );

	screen_size = linewidth * vinfo.yres;
	screen = mmap(0, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fbdev, 0);

	cmap.start = 0;
	cmap.len = 256;
	cmap.red = malloc( sizeof(__u16) * 256 );
	cmap.green = malloc( sizeof(__u16) * 256 );
	cmap.blue = malloc( sizeof(__u16) * 256 );
	cmap.transp = NULL;

	if ( ioctl( fbdev, FBIOGETCMAP, &cmap ) == -1 ) {
		printf( "fbdev: error getting cmap: %s\n", strerror(errno) );
	}

	IO_SetRenderRes( rWidth, rHeight );

	benchmark_time = clock();

	return 0;
}

void IO_SetEmuName( const char* name ) {
	emuName = name;
} 

void IO_SetTitle( const char* title ) {
	(void)title;
}

void IO_DrawPixel8( int x, int y, unsigned char color ) {
	y += yAdjust;

	int l = (linewidth * y) + (x * 2) + xAdjust;
	screen[l++] = screen[l+1] = color;
}

void IO_DrawPixel24( int x, int y, uint8_t r, uint8_t g, uint8_t b ) {
	(void)x;
	(void)y;
	(void)r;
	(void)g;
	(void)b;
	/*
	screen[(renderWidth * y * 3) + (x * 3) + 0] = r;
	screen[(renderWidth * y * 3) + (x * 3) + 1] = g;
	screen[(renderWidth * y * 3) + (x * 3) + 2] = b;
	*/
}

void IO_ScreenCopy( void ) {
	/* not needed - renders directly to screen */
}

void IO_SetKeyPressCallback( void (*Callback)( int key ) ) {
	KeyPressCallback = Callback;
}

void IO_SetKeyReleaseCallback( void (*Callback)( int key ) ) {
	KeyReleaseCallback = Callback;	
}

int IO_Update( sm83_t * cpu ) {
	int r = 1;

	/*printf( "begin video update %i\n", updates++ );*/
	IO_ScreenCopy();
	
	if ( benchmark_limit > 0 ) {
		if ( ++benchmark == benchmark_limit ) {
			clock_t end = clock();
			r = 0;

			printf( "%i frames in %f cpu secs\n", benchmark, ((double)(end - benchmark_time)) / CLOCKS_PER_SEC );
		}
	}

	if ( stop ) {
		printf("signal caught; exiting");
		r = 0;
	}

	return r;
}

void IO_SetPaletteColor( int i, unsigned char r, unsigned char g, unsigned char b ) {
	cmap.red[i] = r << 8;
	cmap.green[i] = g << 8;
	cmap.blue[i] = b << 8;

	printf("fbdev: set color %i to %i,%i,%i\n", i, r, g, b);

	if ( ioctl( fbdev, FBIOPUTCMAP, &cmap ) == -1 ) {
                printf( "fbdev: error setting cmap: %s\n", strerror(errno) );
        }
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



