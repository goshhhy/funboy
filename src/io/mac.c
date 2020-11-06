#include <Types.h>
#include <Memory.h>
#include <Quickdraw.h>
#include <Fonts.h>
#include <Events.h>
#include <Menus.h>
#include <Windows.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <OSUtils.h>
#include <ToolUtils.h>
#include <SegLoad.h>

#include <stdio.h>
#include <stdlib.h>

#include "gb.h"
#include "io.h"

Rect windRect;
WindowPtr	mainPtr;

RGBColor * screen = NULL;
int renderWidth, renderHeight;

void Initialize(void);

void main(void)
{
	Initialize();
	gb_main( "cpu_instrs.gb" );
}

void Initialize(void) {
	RGBColor bg, fg;

	OSErr		error;
	SysEnvRec	theWorld;

	error = SysEnvirons(1, &theWorld);
	
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();

	windRect.top = 50;
	windRect.left = 50;
	windRect.bottom = 50 + 144;
	windRect.right = 50 + 160;
	mainPtr = NewCWindow(nil, &windRect, "\pfunboy!", true, noGrowDocProc, 
						(WindowPtr) -1, false, 0);
		
	SetPort(mainPtr);
	MoveTo(0, 0);
	
	fg.red = 0;
	fg.green = 0;
	fg.blue = 0;
	bg.red = 255;
	bg.green = 255;
	bg.blue = 255;
	
	RGBForeColor(&fg);
	RGBBackColor(&bg);
}

int IO_Init( int vWidth, int vHeight, int rWidth, int rHeight ) {
	IO_SetRenderRes( rWidth, rHeight );
	return 0;
}

int IO_Update( void ) {
	int eventMask = mDownMask | mUpMask | keyDownMask | keyUpMask | updateMask | activMask;
	EventRecord e;
	WindowPtr window;
	
	printf("io_update()\n");
	
	SystemTask();
	while( GetNextEvent( everyEvent, &e ) ) {
		switch ( e.what ) {
			case mouseDown:
				switch ( FindWindow( e.where, &window ) ) {
					case inSysWindow: {
						SystemClick(&e, window);
						break;
					}
					case inMenuBar: {
						break;
					}
					case inContent: {
						if ( window != FrontWindow()) {
							SelectWindow( window );
						}
					}
					case inDrag: {
						windRect = qd.screenBits.bounds;
						DragWindow( window, e.where, &windRect );
						break;
					}
				}
				break;
			case updateEvt: {
				int x, y;
				Rect r = qd.thePort->portRect;
				
				
				BeginUpdate( (WindowPtr)e.message );
				SetPort(mainPtr);
				EraseRect(&r);
				for ( y = 0; y < renderHeight; y++ ) {
					for ( x = 0; x < renderWidth; x++ ) {
						SetCPixel( x, y, &screen[ ( y * renderWidth ) + x ] );
					}
				}
				
				EndUpdate( (WindowPtr)e.message );
				break;
			}
			case mouseUp:
				break;
		}		
	}
	
	printf("end io_update()\n");
	
	return 1;
}

void IO_SetRenderRes( int x, int y ) {
	renderWidth = x;
	renderHeight = y;
	
	if ( screen )
		free( screen );
	
	screen = malloc( sizeof( RGBColor ) * x * y );

	printf("render res set to %d by %d", x, y);

	return;
}

void IO_DrawPixel24( int x, int y, unsigned char r, unsigned char g, unsigned char b ) {
	RGBColor c;
	
	if ( !screen ) 
		return;
	
	c.red = r << 8;
	c.green = g << 8;
	c.blue = b << 8;
	
	if ( ( x < 0 ) || ( y < 0 ) || ( x >= renderWidth ) || ( y >= renderHeight ) ) {
		printf("bad pixel! [%i,%i]\n", x, y);
	}
	
	screen[ ( y * renderWidth ) + x ] = c;
	
	return;
}

void IO_SetBg( unsigned char r, unsigned char g, unsigned char b ) {
	return;
}

void IO_SetKeyPressCallback( void (*Callback)( int key ) ) {
	return;
}

void IO_SetKeyReleaseCallback( void (*Callback)( int key ) ) {
	return;
}

void IO_SetTitle( const char* title ) {
	return;
}

void IO_SetEmuName( const char* name ) {
	return;
}
