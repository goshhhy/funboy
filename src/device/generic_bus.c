#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

#include "device.h"

#define MAX_MAPPINGS 256

typedef struct busMapping_s {
    char *name;
    busAddress_t addr_start;
    busAddress_t addr_end;
    busDevice_t* device;
} busMapping_t;

typedef struct busInfo_s {
    char *name;
    char emptyVal;
#ifdef BUS_MAP_FAST16
    /* pointer to an array of 65536 busMapping_t* */
    busMapping_t ** mappings;
#else
    busMapping_t mappings[MAX_MAPPINGS];
#endif
} busInfo_t;


#ifdef BUS_MAP_FAST16
static busMapping_t* GetTargetBusMapping( busInfo_t* bus, busAddress_t addr ) {
    return bus->mappings[addr];
}
#else
static busMapping_t* GetTargetBusMapping( busInfo_t* bus, busAddress_t addr ) {
    int i;

    for ( i = 0; i < MAX_MAPPINGS; i++ ) {
        if ( addr >= bus->mappings[i].addr_start ) {
            if ( addr <= bus->mappings[i].addr_end ) {
                return &bus->mappings[i];
            }
        }
    }
    printf( "bus %s: no device found for access (offset %08lx)\n", bus->name, addr );
    return NULL;
}
#endif

static unsigned char GenericBusRead( busDevice_t *dev, busAddress_t addr, int final ) {
    busInfo_t *bus;
    busMapping_t *mapping;

    #ifdef BUS_MAP_PARANOID
    if ( !dev || !dev->data )
        return 0;
    #endif

    bus = dev->data;

    mapping = GetTargetBusMapping( bus, addr );
    if ( !mapping || mapping->device == dev )
        return bus->emptyVal;
    return mapping->device->Read8( mapping->device, addr - mapping->addr_start, final );
}


static void GenericBusWrite( busDevice_t *dev, busAddress_t addr, unsigned char val, int final ) {
    busInfo_t *bus;
    busMapping_t *mapping;

    #ifdef BUS_MAP_PARANOID
    if ( !dev || !dev->data )
        return;
    #endif

    bus = dev->data;
    mapping = GetTargetBusMapping( bus, addr );
    if ( !mapping || mapping->device == dev )
        return;
    mapping->device->Write8( mapping->device, addr - mapping->addr_start, val, final );
}

void GenericBusSetEmptyVal( busDevice_t *dev, unsigned char val ) {
    busInfo_t *bus;

    if ( !dev || !dev->data )
        return;

    bus = dev->data;
    bus->emptyVal = val;
}

#ifdef BUS_MAP_FAST16
void GenericBusMapping( busDevice_t *dev, char* name, busAddress_t addr_start, busAddress_t addr_end, busDevice_t *subdev ) {
    busInfo_t *bus;
    busMapping_t *mapping;
    int i;

    if ( !dev || !dev->data || !subdev || ( addr_start > addr_end ) )
        return;

    bus = dev->data;

    mapping = malloc( sizeof( busMapping_t ) );
    if ( !mapping )
        return;

    mapping->name = name;
    mapping->addr_start = addr_start;
    mapping->addr_end = addr_end;
    mapping->device = subdev;

    for ( i = addr_start; i <= addr_end; i++ )
        bus->mappings[i] = mapping;

    printf( "added device to %s bus as \"%s\"\n", bus->name, name );
}
#else
void GenericBusMapping( busDevice_t *dev, char* name, busAddress_t addr_start, busAddress_t addr_end, busDevice_t *subdev ) {
    busInfo_t *bus;
    int i;

    if ( !dev || !dev->data || !subdev || ( addr_start > addr_end ) )
        return;
    bus = dev->data;
    for ( i = 0; i < MAX_MAPPINGS; i++ ) {
        if ( bus->mappings[i].device == NULL ) {
            bus->mappings[i].name = name;
            bus->mappings[i].addr_start = addr_start;
            bus->mappings[i].addr_end = addr_end;
            bus->mappings[i].device = subdev;
            printf( "added device to %s bus as \"%s\"\n", bus->name, name );
            return;
        }
    }
    fprintf( stderr, "error: too many bus mappings\n" );
}
#endif

busDevice_t *GenericBus( char* name ) {
    busDevice_t *dev;
    busInfo_t *bus;

    if ( !( dev = malloc( sizeof( busDevice_t ) ) ) || !( bus = calloc( 1, sizeof( busInfo_t ) ) ) ) {
        fprintf( stderr, "couldn't allocate ram block for system bus information\n" );
        return NULL;
    }

    #ifdef BUS_MAP_FAST16
    if ( !(bus->mappings = malloc( 65536 * sizeof(void*) ) ) ) {
        fprintf( stderr, "couldn't allocate ram for fast bus mapping\n" );
    }
    memset( bus->mappings, 0, 65536 * sizeof(void*) );
    #endif

    bus->name = name;
    dev->data = bus;
    dev->Read8 = GenericBusRead;
    dev->Write8 = GenericBusWrite;
    printf( "created %s system bus\n", name );
    return dev;
}
