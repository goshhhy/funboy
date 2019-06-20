
typedef struct busDevice_s {
    void* data;
    uint8_t (*Read8)( struct busDevice_s* self, uint32_t addr );
    uint8_t (*Write8)( struct busDevice_s* self, uint32_t addr, uint8_t val );
} busDevice_t;