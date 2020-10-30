int IO_Init( int wWidth, int wHeight, int rWidth, int rHeight );
int IO_Update( void );
void IO_SetRenderRes( int x, int y );
void IO_DrawPixel( int x, int y, unsigned char r, unsigned char g, unsigned char b );
void IO_SetBg( unsigned char r, unsigned char g, unsigned char b );
void IO_SetKeyPressCallback( void (*Callback)( int key ) );
void IO_SetKeyReleaseCallback( void (*Callback)( int key ) );
void IO_SetTitle( const char* title );
void IO_SetEmuName( const char* name );