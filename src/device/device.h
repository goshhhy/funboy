
typedef struct busDevice_s {
    void* data;
    uint8_t (*Read8)( struct busDevice_s* self, uint32_t addr );
    void (*Write8)( struct busDevice_s* self, uint32_t addr, uint8_t val );
} busDevice_t;

// generic devices for all platforms
busDevice_t *GenericRam( size_t len );
busDevice_t *GenericRom( char *fileName, size_t len );
busDevice_t *GenericBus( char* name );
void GenericBusMapping( busDevice_t *dev, char* name, uint32_t addr_start, uint32_t addr_end, busDevice_t *subdev );
