/***************************************************************************//**
  @file     LEDs.h
  @brief    LEDs driver
  @author   Grupo 4
 ******************************************************************************/

#ifndef _LEDS_H_
#define _LEDS_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdbool.h>
#include "stdint.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define NO_LED 0b00
#define LED1 0b01
#define LED2 0b10
#define LED3 0b11

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/


/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialize the LEDs
 * @return return true if the LEDS were initialized successfully
 */
bool LEDS_Init(void);


/**
 * @brief Turn on the LEDS
 * @param ledNumber the number of the LED to turn on
 */
void LEDS_On(uint8_t ledNumber);


/*******************************************************************************
 ******************************************************************************/

#endif // _LEDS_H_