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
#include <Palettes.h>

#include <stdio.h>
#include <stdlib.h>

#include "gb.h"
#include "io.h"

#define rMenuBar 128

enum {
	mApple = 128,
	mFile,
	mEmulation,
};

enum {
	iAbout = 1,
};

enum {
	iOpen = 1,
	iClose,
	
	iQuit = 4,
};

enum {
	iPause = 1,
	iReset,

	iTurbo = 4,
};

Rect 			gbRect, windRect;
WindowPtr		mainPtr;

char * 			screen = NULL;
int 			renderWidth, renderHeight;

PaletteHandle 	palette;
PixMapHandle 	pixmap;

static int gRun = 1;

void Initialize(void);

void main(void)
{
	Initialize();
	gb_main( "zelda.gb" );
}

void IO_SetPaletteColor( int index, unsigned char r, unsigned char g, unsigned char b ) {
	RGBColor c;
	
	c.red = r << 8;
	c.green = g << 8;
	c.blue = b << 8;

	SetEntryColor( palette, index, &c );
	Palette2CTab(palette, (**pixmap).pmTable);
	return;
}

void Initialize(void) {
	RGBColor bg, fg;
	OSErr		error;
	SysEnvRec	theWorld;
	Handle 			menuBar;


	error = SysEnvirons(1, &theWorld);
	
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();

	// set up menus
	menuBar = GetNewMBar(rMenuBar);
	if ( menuBar ) {
		SetMenuBar( menuBar );
		DisposeHandle( menuBar );
		AppendResMenu( GetMenuHandle(mApple), 'DRVR'); // set up apple menu items
		DrawMenuBar();
	}

	palette = NewPalette( 192, nil, pmTolerant, 0x5000 );

	windRect.top = 50;
	windRect.left = 50;
	windRect.bottom = 50 + 144;
	windRect.right = 50 + 160;
	mainPtr = NewCWindow(nil, &windRect, "\pfunboy!", true, noGrowDocProc, 
						(WindowPtr) -1, false, 0);
		
	SetPort(mainPtr);
	MoveTo(0, 0);
	
	pixmap = NewPixMap();

	fg.red = 0;
	fg.green = 0;
	fg.blue = 0;
	bg.red = 255;
	bg.green = 255;
	bg.blue = 255;
	
	RGBForeColor(&fg);
	RGBBackColor(&bg);

	printf("start\n");
}

int IO_Init( int vWidth, int vHeight, int rWidth, int rHeight ) {
	IO_SetRenderRes( rWidth, rHeight );
	return 0;
}

static int IO_UpdateScreen( void ) {
	int x, y;
	Rect r = qd.thePort->portRect;
	WindowPtr wind = (WindowPtr)mainPtr;

	BeginUpdate( wind );
	SetPort( mainPtr );

	ForeColor( blackColor );
	BackColor( whiteColor );

	CopyBits( *pixmap, *(((CGrafPtr)wind)->portPixMap),
			&gbRect, &(wind->portRect),
			srcCopy, NULL);

	EndUpdate( (WindowPtr)wind );
}

static void IO_HandleAppleMenuItem( short item ) {
	Str255 itemName;

	if ( item == iAbout ) {
		; // todo: show about window
	} else {
		GetMenuItemText(GetMenuHandle(mApple), item, itemName);
		OpenDeskAcc(itemName);
	}
}

static void IO_HandleFileMenuItem( short item ) {
	switch( item ) {
		case iQuit:
			gRun = 0;
			break;
		default:
			break;
	}
}
static void IO_HandleEmulationMenuItem( short item ) {}


static void IO_HandleMenuItem( long menuitem ) {
	short menu = (menuitem >> 16);
	short item = (menuitem & 0xffff);

	switch( menu ) {
		case mApple:
			IO_HandleAppleMenuItem( item );
			break;
		case mFile:
			IO_HandleFileMenuItem( item );
			break;
		case mEmulation:
			IO_HandleEmulationMenuItem( item );
			break;
		default:
			return;
	}

	HiliteMenu(0);
}

int IO_Update( void ) {
	int eventMask = mDownMask | mUpMask | keyDownMask | keyUpMask | updateMask | activMask;
	EventRecord e;
	WindowPtr window;
	static int updcount = 0;
		
	//printf("IO_Update()");

	SystemTask();
	InvalRect( &qd.thePort->portRect );
	IO_UpdateScreen();
	while( GetNextEvent( everyEvent, &e ) ) {
		switch ( e.what ) {
			case mouseDown:
				switch ( FindWindow( e.where, &window ) ) {
					case inSysWindow: {
						SystemClick(&e, window);
						break;
					}
					case inMenuBar: {
						IO_HandleMenuItem( MenuSelect( e.where ) );
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
					IO_UpdateScreen();
				}
				break;
			case mouseUp:
				break;
		}		
	}
		
	return gRun;
}

void IO_SetRenderRes( int x, int y ) {
	renderWidth = x;
	renderHeight = y;

	if ( screen )
		free( screen );
	
	screen = malloc( sizeof( char ) * x * y );

	gbRect.top = 0;
	gbRect.left = 0;
	gbRect.right = x;
	gbRect.bottom = y;

	(**pixmap).bounds = gbRect;
	(**pixmap).pixelSize = 8;
	(**pixmap).cmpSize = 8;
	(**pixmap).pixelType = 0;
	(**pixmap).cmpCount = 1;
	(**pixmap).packType = 0;
	(**pixmap).planeBytes = 0;
	(**pixmap).pmVersion = 0;
	(**pixmap).rowBytes = x | 0x8000;
	(**pixmap).baseAddr = screen;

	Palette2CTab(palette, (**pixmap).pmTable);

	printf("render res set to %d by %d", x, y);

	return;
}

void IO_DrawPixel24( int x, int y, unsigned char r, unsigned char g, unsigned char b ) {
	/*RGBColor c;
	
	if ( !screen ) 
		return;
	
	c.red = r << 8;
	c.green = g << 8;
	c.blue = b << 8;
	
	if ( ( x < 0 ) || ( y < 0 ) || ( x >= renderWidth ) || ( y >= renderHeight ) ) {
		printf("bad pixel! [%i,%i]\n", x, y);
	}
	
	screen[ ( y * renderWidth ) + x ] = c;
	*/
	return;
}

void IO_DrawPixel8( int x, int y, unsigned char c ) {
	if ( !screen ) 
		return;
	
	if ( ( x < 0 ) || ( y < 0 ) || ( x >= renderWidth ) || ( y >= renderHeight ) ) {
		printf("bad pixel! [%i,%i]\n", x, y);
	}
	
	screen[ ( y * renderWidth ) + x ] = c;
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
