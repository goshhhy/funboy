#include <stdio.h>
#include <stdlib.h> 

#include "device.h"

typedef struct romInfo_s {
    char* name;
    size_t len;
    char* bytes;
    int has_ram;
    int has_battery;
    int rom_bank;
    int ram_bank;
    size_t rom_bank_offset;
    int ram_select;
} romInfo_t;

static unsigned char MBC1RomRead( busDevice_t *dev, busAddress_t addr, int final ) {
    romInfo_t *rom;

    if ( !dev || !dev->data )
        return 0;
    rom = dev->data;

    if ( addr >= 0x8000 ) {
        fprintf( stderr, "warning: MBC1RomRead: address out of bounds [%s:%04lx]\n", rom->name, addr );
        return 0;
    }
    if ( addr < 0x4000 )
        return rom->bytes[addr];
    
    return rom->bytes[( addr - 0x4000 ) + rom->rom_bank_offset];
}


static void MBC1RomWrite( busDevice_t *dev, busAddress_t addr, unsigned char val, int final ) {
    romInfo_t *rom;

    if ( !dev || !dev->data )
        return;
    rom = dev->data;

    if ( addr > 0x8000 ) {
        fprintf( stderr, "warning: MBC1RomWrite: address out of bounds\n" );
        exit(1);
        return;
    }

    if ( addr < 0x2000 ) {
        /* enable/disable cartram */
    } else if ( addr < 0x4000 ) {
        /* set lower bits of rom bank number */
        rom->rom_bank &= 0x60;
        if ( ( val & 0x1f ) == 0 ) {
            rom->rom_bank |= 0x01;
        } else {
            rom->rom_bank |= ( val & 0x1f );
        }
        rom->rom_bank_offset = rom->rom_bank * 0x4000;
    } else if ( addr < 0x6000 ) {
        /* set cartram bank number OR upper bits of rom bank number */
        if ( rom->ram_select ) {
            rom->ram_bank = val & 0x03;
        } else {
            /* set upper rom bank bits */
            rom->rom_bank &= 0x1F;
            rom->rom_bank |= ( ( val & 0x3 ) << 5 );

            rom->rom_bank_offset = rom->rom_bank * 0x4000;
        }
    } else if ( addr < 0x8000 ) {
        /* rom/ram mode select */
        rom->ram_select = ( ( val & 0x1 ) == 1 );
    }
}

char* MBC1RomBytesPtr( busDevice_t *dev ) {
    romInfo_t *rom;

    if ( !dev || !dev->data )
        return NULL;
    rom = dev->data;

    return rom->bytes;
}

busDevice_t *MBC1Rom( char *fileName, size_t len ) {
    FILE *f = fopen( fileName, "r" );
    busDevice_t *dev;
    romInfo_t *rom;
    size_t rem;

    if ( !f ) {
        fprintf( stderr, "couldn't open rom %s\n", fileName );
        return NULL;
    }
    if ( !( dev = malloc( sizeof( busDevice_t ) ) ) || !( rom = malloc( sizeof( romInfo_t ) ) ) ) {
        fprintf( stderr, "couldn't allocate ram for %s\n", fileName );
        fclose( f );
        return NULL;
    }
    rom->name = fileName;
    rom->len = len;
    rom->bytes = calloc( 1, len );
    rom->rom_bank = 0;
    rom->rom_bank_offset = 0x4000;
    rom->ram_bank = 0;
    rom->has_ram = 1;
    rom->has_battery = 1;
    
    dev->data = rom;
    dev->Read8 = MBC1RomRead;
    dev->Write8 = MBC1RomWrite;
    for ( rem = len; rem > 0; ) {
        size_t read = fread( rom->bytes, 1, rem, f );
        rem = rem - read;
        if ( read == 0 ) {
            fprintf( stderr, "warning: MBC1Rom: hit eof with %lu bytes remaining while loading %s\n"\
                     "                     (check if file is correct)\n", rem, fileName );
            break;
        }
    }
    printf( "mbc1: created rom device from %s\n", fileName );
    fclose( f );
    return dev;
}