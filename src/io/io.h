int IO_Init( int wWidth, int wHeight, int rWidth, int rHeight );
bool IO_Update( void );
void IO_SetRenderRes( int x, int y );
void IO_DrawPixel( int x, int y, uint8_t r, uint8_t g, uint8_t b );
void IO_SetBg( uint8_t r, uint8_t g, uint8_t b );
void IO_SetKeyPressCallback( void (*Callback)( int key ) );
void IO_SetKeyReleaseCallback( void (*Callback)( int key ) );
void IO_SetTitle( const char* title );
void IO_SetEmuName( const char* name );