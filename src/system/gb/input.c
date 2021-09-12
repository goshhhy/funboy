#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "version.h"
#include "device.h"
#include "sm83.h"
#include "io.h"
#include "input.h"

typedef struct regInfo_s {
    char* name;
    size_t len;
    unsigned long* data;
} regInfo_t;

static int buttonsSelected = 0;
static int padsSelected = 0;
static unsigned char padStatus = 0xff;
static unsigned char buttonStatus = 0xff;

static void JoyRegisterWrite( busDevice_t *dev, busAddress_t addr, unsigned char val, int final ) {
    if ( ( val & 0x20 ) == 0 ) {
        buttonsSelected = 1;
    } else {
        buttonsSelected = 0;
    }
    if ( ( val & 0x10 ) == 0 ) {
        padsSelected = 1;
    } else {
        padsSelected = 0;
    }
}

static unsigned char JoyRegisterRead( busDevice_t *dev, busAddress_t addr, int final ) {
    unsigned char r = 0;


    if ( buttonsSelected ) {
        r |= buttonStatus;
    } 
    if ( padsSelected ) {
        r |= padStatus;
    }
    return r;
}

static void KeyPressCallback( int key ) {
    /*printf( "key %i pressed\n", key );*/
    switch ( key ) {
        case 79: /* right*/
            padStatus = padStatus & 0xfe;
            break;
        case 80: /* left*/
            padStatus = padStatus & 0xfd;
            break;
        case 82: /* up*/
            padStatus = padStatus & 0xfb;
            break;
        case 81: /* down*/
            padStatus = padStatus & 0xf7;
            break;
        case 27: /* a*/
            buttonStatus = buttonStatus & 0xfe;
            break;
        case 29: /* b*/
            buttonStatus = buttonStatus & 0xfd;
            break;
        case 229: /* select*/
            buttonStatus = buttonStatus & 0xfb;
            break;
        case 40: /* start*/
            buttonStatus = buttonStatus & 0xf7;
            break;
        default:
            break;
    }
}

static void KeyReleaseCallback( int key ) {
    /*printf( "key %i released\n", key );*/
    switch ( key ) {
        case 79: /* right*/
            padStatus = padStatus | 0x01;
            break;
        case 80: /* left*/
            padStatus = padStatus | 0x02;
            break;
        case 82: /* up*/
            padStatus = padStatus | 0x04;
            break;
        case 81: /* down*/
            padStatus = padStatus | 0x08;
            break;
        case 27: /* a*/
            buttonStatus = buttonStatus | 0x01;
            break;
        case 29: /* b*/
            buttonStatus = buttonStatus | 0x02;
            break;
        case 229: /* select*/
            buttonStatus = buttonStatus | 0x04;
            break;
        case 40: /* start*/
            buttonStatus = buttonStatus | 0x08;
            break;
        default:
            break;
    }
}

gbInput_t *GbInput( busDevice_t *bus, sm83_t *cpu ) {
    gbInput_t *input = malloc( sizeof( gbInput_t ) );
    input->cpu = cpu;

    IO_SetKeyPressCallback( KeyPressCallback );
    IO_SetKeyReleaseCallback( KeyReleaseCallback );

    GenericBusMapping( bus, "Joy", 0xFF00, 0xFF00, GenericRegister( "Joy", NULL, 1, JoyRegisterRead, JoyRegisterWrite ) );
    return input;
}

