
typedef struct busDevice_s {
    void* data;
    uint8_t (*Read8)( struct busDevice_s* self, uint32_t addr, bool final );
    void (*Write8)( struct busDevice_s* self, uint32_t addr, uint8_t val, bool final );
} busDevice_t;

// generic devices for all platforms
busDevice_t *GenericRam( size_t len );
busDevice_t *GenericRom( char *fileName, size_t len );
char* GenericRomBytesPtr( busDevice_t *dev );
busDevice_t *GenericBus( char* name );
void GenericBusMapping( busDevice_t *dev, char* name, uint32_t addr_start, uint32_t addr_end, busDevice_t *subdev );
void GenericBusSetEmptyVal( busDevice_t *dev, uint8_t val );
busDevice_t *GenericRegister( char *name, uint8_t *data, size_t len, uint8_t (*Read8)( struct busDevice_s* self, uint32_t addr, bool final ),
                                    void (*Write8)( struct busDevice_s* self, uint32_t addr, uint8_t val, bool final ) );