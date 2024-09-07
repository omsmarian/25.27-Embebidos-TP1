/***************************************************************************//**
  @file     LEDs.h
  @brief    LEDs driver
  @author   Group 4
 ******************************************************************************/

#ifndef _LEDS_H_
#define _LEDS_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdbool.h>
#include <stdint.h>


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialize the LEDs
 * @return return true if the LEDS were initialized successfully
 */
bool LEDS_Init(void);

/**
 * @brief indicates which LEDs to turn on
 * @param num is a binari number of 3 bits. 
 */
void LEDS_Set(uint8_t num);


/*******************************************************************************
 ******************************************************************************/

#endif // _LEDS_H_
