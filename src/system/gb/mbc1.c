#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "device.h"
#include "sm83.h"
#include "alarms.h"

typedef struct romInfo_s {
    char* name;
    size_t len;
    char* bytes;

    alarmManager_t * alarmManager;
    alarm_t alarm;

    int has_ram;
    int has_battery;
    int rom_bank;
    int ram_bank;
    size_t rom_bank_offset;

    FILE *saveFp;
    char* ramFilename;
    char* ramBytes;
    int ram_select;
    int ram_enabled;

    sm83_t* cpu;
} romInfo_t;

static unsigned char MBC1RomRead( busDevice_t *dev, busAddress_t addr, int final ) {
    romInfo_t *rom;

    if ( !dev || !dev->data )
        return 0;
    rom = dev->data;

    if ( addr >= 0x8000 ) {
        fprintf( stderr, "warning: MBC1RomRead: address out of bounds [%s:%04x]\n", rom->name, addr );
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
        if ( ( val & 0x0F ) == 0x0A ) {
            rom->ram_enabled = 1;
        } else {
            rom->ram_enabled = 0;
        }
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
    Sm83_InvalidateBankingRomCache( rom->cpu );

}

static void MBC1RamSaveCallback( void * data ) {
    romInfo_t *rom = data;

    rewind( rom->saveFp );
    if ( fwrite( rom->ramBytes, 0x1, 0x8000, rom->saveFp ) < 0x8000 ) {
        fprintf( stderr, "MBC1RamSaveCallback: error writing save file\n" );
    } else {
        printf( "MBC1RamSaveCallback: save complete\n" );
    }

    rom->alarm.when = -1;
}

static unsigned char MBC1RamRead( busDevice_t *dev, busAddress_t addr, int final ) {
    romInfo_t *rom;

    if ( !dev || !dev->data )
        return 0;
    rom = dev->data;

    if ( addr >= 0x2000 ) {
        fprintf( stderr, "warning: MBC1RomRead: address out of bounds [%s:%04x]\n", rom->name, addr );
        return 0;
    }
    return rom->ramBytes[ (0x2000 * (rom->ram_bank & 0x03)) + addr ];
}

static void MBC1RamWrite( busDevice_t *dev, busAddress_t addr, unsigned char val, int final ) {
    romInfo_t *rom;

    if ( !dev || !dev->data )
        return;
    rom = dev->data;

    if ( addr >= 0x2000 ) {
        fprintf( stderr, "warning: MBC1RomRead: address out of bounds [%s:%04x]\n", rom->name, addr );
        return;
    }
    rom->ramBytes[ (0x2000 * rom->ram_bank) + addr ] = val;

    /* we don't actually care much when this happens, just that it happens in the future,
    so there's really no need to alert the alarm manager that we changed anything - just
    set the time to the future */
    rom->alarm.when = 1000;
}

char* MBC1RomBytesPtr( busDevice_t *dev ) {
    romInfo_t *rom;

    if ( !dev || !dev->data )
        return NULL;
    rom = dev->data;

    return rom->bytes;
}

busDevice_t *MBC1Rom( char *fileName, size_t len, busDevice_t** _ram, alarmManager_t *alarmManager, sm83_t * cpu ) {
    FILE *f = fopen( fileName, "r" );
    busDevice_t *dev;
    busDevice_t *ram;
    romInfo_t *rom;
    char * p;
    size_t rem;

    if ( !f ) {
        fprintf( stderr, "couldn't open rom %s\n", fileName );
        return NULL;
    }
    if ( !( dev = malloc( sizeof( busDevice_t ) ) ) ||
         !( ram = malloc( sizeof( busDevice_t ) ) ) ||
         !( rom = malloc( sizeof( romInfo_t ) ) ) )  {
        fprintf( stderr, "couldn't allocate ram for %s\n", fileName );
        fclose( f );
        return NULL;
    }
    rom->name = fileName;

    rom->ramFilename = malloc( strlen(fileName) + 5);
    strcpy(rom->ramFilename, fileName);
    p = strrchr(rom->ramFilename, '.');
    if (p) {
        *p = '\0';
    }
    strcat( rom->ramFilename, ".sav" );
    rom->ramBytes = malloc( 0x8000 );
    rom->saveFp = fopen( rom->ramFilename, "r+" );
    if ( rom->saveFp ) {
        for ( rem = 0x8000; rem > 0; ) {
            size_t read = fread( rom->ramBytes, 1, rem, rom->saveFp );
            rem = rem - read;
            if ( read == 0 ) {
                fprintf( stderr, "warning: MBC1Rom: hit eof with %lu bytes remaining while loading %s\n"\
                        "                     (check if file is correct)\n", rem, rom->ramFilename );
                break;
            }
        }
        printf( "loaded save file %s\n", rom->ramFilename );
    } else {
        rom->saveFp = fopen( rom->ramFilename, "w+" );
        printf( "created new save file %s\n", rom->ramFilename );
    }

    rom->len = len;
    rom->bytes = calloc( 1, len );
    rom->rom_bank = 0;
    rom->rom_bank_offset = 0x4000;
    rom->ram_bank = 0;
    rom->has_ram = 1;
    rom->has_battery = 1;
    rom->ram_enabled = 0;
    
    rom->alarmManager = alarmManager;
    rom->alarm.name = "saveAlarm";
    rom->alarm.when = 1000;
    rom->alarm.Callback = MBC1RamSaveCallback;
    rom->alarm.callbackData = rom;

    AlarmAdd( alarmManager, &rom->alarm );

    rom->cpu = cpu;

    dev->data = rom;
    dev->Read8 = MBC1RomRead;
    dev->Write8 = MBC1RomWrite;

    ram->data = rom;
    ram->Read8 = MBC1RamRead;
    ram->Write8 = MBC1RamWrite;

    *_ram = ram;

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