/***************************************************************************//**
  @file     LEDs.c
  @brief    LEDs driver
  @author   Grupo 4
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "LEDs.h"
#include "board.h"
#include "gpio.h"
#include "pisr.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define NO_LED 0b00
#define LED1 0b11
#define LED2 0b01
#define LED3 0b10

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/


/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

/**
 * @brief Refresh the LEDS
 */
void LEDS_refresh(void);

/**
 * @brief Turns on only one LED
 * @param ledNumber which LED to turn on
 */
void LEDS_On(uint8_t ledNumber);

/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static uint8_t ledStatus;
static uint8_t counter;

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

bool LEDS_Init(void)
{
    gpioWrite(PIN_LED_EXT_STATUS0, LOW);
    gpioWrite(PIN_LED_EXT_STATUS0, LOW);
    gpioMode(PIN_LED_EXT_STATUS0, OUTPUT);
    gpioMode(PIN_LED_EXT_STATUS1, OUTPUT);
    
    pisrRegister(LEDS_refresh, 7);    // 7 ms

    return true;
}

void LEDS_Set(uint8_t num)
{
    ledStatus = num;
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void LEDS_On(uint8_t ledNumber)
{
    gpioWrite(PIN_LED_EXT_STATUS0, ledNumber & 0b01);
    gpioWrite(PIN_LED_EXT_STATUS1, ledNumber & 0b10);
}

void LEDS_refresh(void)
{
    if((ledStatus & 0b100) && (counter == 0))
        LEDS_On(LED3);
    else if((ledStatus & 0b010) && (counter == 1))
        LEDS_On(LED2);
    else if((ledStatus & 0b001) && (counter == 2))
        LEDS_On(LED1);
    else
        LEDS_On(NO_LED);
    counter++;
}

/******************************************************************************/
