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

#define DISPLAY_DIGITS			4
#define DISPLAY_SEGMENTS		8

#define DISPLAY_FREQUENCY_HZ	50

#define DISPLAY_MIN_BRIGHTNESS	0
#define DISPLAY_MAX_BRIGHTNESS	100

#define DISPLAY_CLEAR       	' '


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialize the display
 * @return Initialization status (true if successful)
 * @note This function must be called before any other display function
 * @example if (DisplayInit()) { // Initialization successful }
 */
bool DisplayInit (void);

/**
 * @brief Write characters to the display
 * @param characters Characters to write (String)
 * @param length Number of characters to write
 * @note If length is greater than DISPLAY_LENGTH, only the first DISPLAY_LENGTH characters will be written
 * @example DisplayWrite("1234", 4); // Write '1234' to the display
 */
void DisplayWrite (char * characters);

/**
 * @brief Set a specific digit in the display
 * @param digit Digit to set
 * @param character Character to write
 * @note Use CLEAR or ' ' to clear the digit
 * @example DisplaySetDigit(0, 'A'); // Write 'A' to the first digit
 */
void DisplaySetDigit (uint8_t digit, char character);

/**
 * @brief Write a number to the display
 * @param number Number to write
 * @note If the number has more than DISPLAY_LENGTH digits, only the last DISPLAY_LENGTH digits will be written
 * @example DisplayWriteNumber(1234); // Write '1234' to the display
 */
void DisplayWriteNumber (uint16_t number);

/**
 * @brief Clear the display
 * @example DisplayClear(); // Clear the display
 */
void DisplayClear (void);

/**
 * @brief Set the brightness of the display
 * @param brightness Brightness level (0-100)
 * @example DisplaySetBrightness(50); // Set the brightness to 50%
 */
void DisplaySetBrightness (uint8_t brightness);


/*******************************************************************************
 ******************************************************************************/

#endif // _DISPLAY_H_
