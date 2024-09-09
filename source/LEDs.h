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
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define ALL             0b111
#define NOTHING         0b000
#define ONLY_LEFT       0b100 
#define ONLY_RIGHT      0b001 
#define ONLY_CENTER     0b010 
#define LEFT_CENTER     0b110 
#define CENTER_RIGHT    0b011 
#define LEFT_RIGHT      0b101 

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
