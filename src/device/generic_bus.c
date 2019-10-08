#include <stdio.h>
#include <stdlib.h> 
#include <stdint.h>
#include <stdbool.h>

#include "device.h"

#define MAX_MAPPINGS 256

typedef struct busMapping_s {
    char *name;
    uint32_t addr_start;
    uint32_t addr_end;
    busDevice_t* device;
} busMapping_t;

typedef struct busInfo_s {
    char *name;
    busMapping_t mappings[MAX_MAPPINGS];
    char emptyVal;
} busInfo_t;

static busMapping_t* GetTargetBusMapping( busInfo_t* bus, uint32_t addr ) {
    for ( int i = 0; i < MAX_MAPPINGS; i++ ) {
        if ( bus->mappings[i].device ) {
            if ( addr >= bus->mappings[i].addr_start ) {
                if ( addr <= bus->mappings[i].addr_end ) {
                    //printf( "bus %s forwarding access at %04x to device %s\n", bus->name, addr, bus->mappings[i].name );
                    return &bus->mappings[i];
                }
            }
        }
    }
    printf( "bus %s: no device found for access (offset %08x)\n", bus->name, addr );
    //exit( 1 );
    return NULL;
}

static uint8_t GenericBusRead( busDevice_t *dev, uint32_t addr, bool final ) {
    busInfo_t *bus;
    busMapping_t *mapping;

    if ( !dev || !dev->data )
        return 0;
    bus = dev->data;

    mapping = GetTargetBusMapping( bus, addr );
    if ( !mapping || mapping->device == dev )
        return bus->emptyVal;
    return mapping->device->Read8( mapping->device, addr - mapping->addr_start, final );
}


static void GenericBusWrite( busDevice_t *dev, uint32_t addr, uint8_t val, bool final ) {
    busInfo_t *bus;
    busMapping_t *mapping;

    if ( !dev || !dev->data )
        return;
    bus = dev->data;
    mapping = GetTargetBusMapping( bus, addr );
    if ( !mapping || mapping->device == dev )
        return;
    mapping->device->Write8( mapping->device, addr - mapping->addr_start, val, final );
}

void GenericBusSetEmptyVal( busDevice_t *dev, uint8_t val ) {
    busInfo_t *bus;

    if ( !dev || !dev->data )
        return;
    bus = dev->data;
    bus->emptyVal = val;
}

void GenericBusMapping( busDevice_t *dev, char* name, uint32_t addr_start, uint32_t addr_end, busDevice_t *subdev ) {
    busInfo_t *bus;
    

    if ( !dev || !dev->data || !subdev || ( addr_start > addr_end ) )
        return;
    bus = dev->data;
    for ( int i = 0; i < MAX_MAPPINGS; i++ ) {
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

busDevice_t *GenericBus( char* name ) {
    busDevice_t *dev;
    busMapping_t *bus;

    if ( !( dev = malloc( sizeof( busDevice_t ) ) ) || !( bus = calloc( 1, sizeof( busInfo_t ) ) )) {
        fprintf( stderr, "couldn't allocate ram block for system bus information\n" );
        return NULL;
    }
    bus->name = name;
    dev->data = bus;
    dev->Read8 = GenericBusRead;
    dev->Write8 = GenericBusWrite;
    printf( "created %s system bus\n", name );
    return dev;
}
