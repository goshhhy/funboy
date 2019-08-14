void Vic_SetCr( uint8_t which, uint8_t val );
uint8_t Vic_GetCr( uint8_t which );
void Vic_SetColorRam( uint16_t offset, uint8_t val );
uint8_t Vic_GetColorRam( uint16_t offset );
void Vic_Init( void );
int Vic_Step( busDevice_t* bus );
busDevice_t *VicVideo( void );