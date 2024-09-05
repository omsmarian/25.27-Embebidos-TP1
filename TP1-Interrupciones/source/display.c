/***************************************************************************//**
  @file     display.c
  @brief    Display driver
  @author   Group 4
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "display.h"
#include "board.h"
#include "gpio.h"
#include "pisr.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define DEVELOPMENT_MODE    1


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void refreshDisplay (void);


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static const uint8_t characters [16] = { 0b00111111,
                                         0b00000110,
                                         0b01011011,
                                         0b01001111,
                                         0b01100110,
                                         0b01101101,
                                         0b01111101,
                                         0b00000111,
                                         0b01111111,
                                         0b01101111 };

// static const uint8_t select [4] = { 0x00, 0x01, 0x02, 0x03 };

static uint8_t display_buffer[DISPLAY_LENGTH] = { 0 };
// static uint8_t display_index = 0;


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

bool DisplayInit (void)
{
    static bool init_flag = false;

    if (!init_flag) // Already initialized
    {
        gpioMode(PIN_DISPLAY_CSEGA,		OUTPUT);
        gpioMode(PIN_DISPLAY_CSEGB,		OUTPUT);
        gpioMode(PIN_DISPLAY_CSEGC,		OUTPUT);
        gpioMode(PIN_DISPLAY_CSEGD,		OUTPUT);
        gpioMode(PIN_DISPLAY_CSEGE,		OUTPUT);
        gpioMode(PIN_DISPLAY_CSEGG,		OUTPUT);
        gpioMode(PIN_DISPLAY_CSEGDP,	OUTPUT);
        gpioMode(PIN_DISPLAY_SEL0,		OUTPUT);
        gpioMode(PIN_DISPLAY_SEL1,		OUTPUT);

        pisrRegister(refreshDisplay, 1000);

        init_flag = true;
    }

    return init_flag;
}

void DisplayWrite (char characters[])
{
    for (uint8_t i = 0; i < DISPLAY_LENGTH; i++)
        display_buffer[i] = characters[i];
}

void DisplayClear (void)
{
    for (uint8_t i = 0; i < DISPLAY_LENGTH; i++)
        display_buffer[i] = 0;
}

void DisplaySetBrightness (uint8_t brightness)
{
    pisrClear(refreshDisplay);
    pisrRegister(refreshDisplay, 1000 / brightness);
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void refreshDisplay (void)
{
    static uint8_t display = 0;
    uint8_t character = characters[display_buffer[display]];

    gpioWrite(PIN_DISPLAY_SEL0,   display   & 0x01);
    gpioWrite(PIN_DISPLAY_SEL1,   display   & 0x02);

    gpioWrite(PIN_DISPLAY_CSEGA,  character & 0x01);
    gpioWrite(PIN_DISPLAY_CSEGB,  character & 0x02);
    gpioWrite(PIN_DISPLAY_CSEGC,  character & 0x04);
    gpioWrite(PIN_DISPLAY_CSEGD,  character & 0x08);
    gpioWrite(PIN_DISPLAY_CSEGE,  character & 0x10);
    gpioWrite(PIN_DISPLAY_CSEGF,  character & 0x20);
    gpioWrite(PIN_DISPLAY_CSEGG,  character & 0x40);
    gpioWrite(PIN_DISPLAY_CSEGDP, character & 0x80);

    display++;
    display %= 4;
}


/******************************************************************************/
