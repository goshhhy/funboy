

/* uncomment for 32-bit bus size */
/* #define BUS_SIZE_32 */

/* uncomment for faster, but more memory intensive, 16-bit bus mapping method */
#define BUS_MAP_FAST16

/* #define BUS_MAP_PARANOID */

#ifdef BUS_SIZE_32
typedef unsigned long busAddress_t;
#else
typedef unsigned short busAddress_t;
#endif

typedef struct busDevice_s {
    void* data;
    unsigned char (*Read8)( struct busDevice_s* self, busAddress_t addr, int final );
    void (*Write8)( struct busDevice_s* self, busAddress_t addr, unsigned char val, int final );
    void * (*DataPtr)( struct busDevice_s* self );
} busDevice_t;

/* generic devices for all platforms */
busDevice_t *GenericRam( size_t len );
busDevice_t *GenericRom( char *fileName, size_t len );
char* GenericRomBytesPtr( busDevice_t *dev );
busDevice_t *GenericBus( char* name );
void GenericBusMapping( busDevice_t *dev, char* name, busAddress_t addr_start, busAddress_t addr_end, busDevice_t *subdev );
void GenericBusSetEmptyVal( busDevice_t *dev, unsigned char val );
busDevice_t *GenericRegister( char *name, unsigned char *data, size_t len, unsigned char (*Read8)( struct busDevice_s* self, busAddress_t addr, int final ),
                                    void (*Write8)( struct busDevice_s* self, busAddress_t addr, unsigned char val, int final ) );
void GenericRegisterReadOnly( busDevice_t *dev, busAddress_t addr, unsigned char val, int final );