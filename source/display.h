/***************************************************************************//**
  @file     display.h
  @brief    Display driver
  @author   Group 4
 ******************************************************************************/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdbool.h>
#include <stdint.h>


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define DISPLAY_LENGTH			4

#define DISPLAY_FREQUENCY_HZ	50

#define MIN_BRIGHTNESS			0
#define MAX_BRIGHTNESS			100


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief
 * @param
 * @return
 */
bool DisplayInit (void);

/**
 * @brief
 * @param
 * @return
 */
void DisplayWrite (uint8_t characters[]);

/**
 * @brief
 * @param
 * @return
 */
void DisplayClear (void);

/**
 * @brief
 * @param
 * @return
 */
void DisplaySetBrightness (uint8_t brightness);


/*******************************************************************************
 ******************************************************************************/

#endif // _DISPLAY_H_
