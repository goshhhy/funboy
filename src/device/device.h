
typedef unsigned long busAddress_t;

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