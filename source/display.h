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

#define DISPLAY_FREQUENCY_HZ	75
#define DISPLAY_ROLL_DELAY_MS	1000

#define DISPLAY_MIN_BRIGHTNESS	0
#define DISPLAY_MAX_BRIGHTNESS	25

#define MAX_STRING_LENGTH		16


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
 * @brief Set a specific digit in the display with a character
 * @param digit Digit to set
 * @param character Character to write
 * @note Use CLEAR or ' ' to clear the digit
 * @example DisplaySetDigit(0, 'A'); // Write 'A' to the first digit
 */
void DisplaySetDigitChar (uint8_t digit, char character);

/**
 * @brief Set a specific digit in the display with a number
 * @param digit Digit to set
 * @param character Number to write
 * @note Use CLEAR or ' ' to clear the digit.
 *       If the digit is grater than DISPLAY_DIGITS or the number is greater than 9, the digit will not be changed
 * @example DisplaySetDigitNum(0, 1); // Write '1' to the first digit
 */
void DisplaySetDigitNum (uint8_t digit, uint8_t number);

/**
 * @brief Write characters to the display
 * @param characters Characters to write (String)
 * @param length Number of characters to write
 * @note If length is greater than DISPLAY_DIGITS, only the first DISPLAY_DIGITS characters will be written
 * @example DisplayWrite("ABCDE"); // Write 'ABCD' to the display
 */
void DisplayWriteChar (char * characters);

/**
 * @brief Write a number to the display
 * @param number Number to write
 * @note If the number has more than DISPLAY_DIGITS digits, only the fisrt DISPLAY_DIGITS digits will be written
 * @example DisplayWriteNumber(1234); // Write '1234' to the display
 */
void DisplayWriteNum (uint8_t n1, uint8_t n2, uint8_t n3, uint8_t n4);

/**
 * @brief Write a string to the display
 * @param string String to write
 * @param start Start position to write the string (first digit is 0)
 * @note If start is greater than DISPLAY_DIGITS, only the last DISPLAY_DIGITS characters will be written
 * @example DisplayWriteString("ABCDE", 3); // Write 'BCDE' to the display
 */
void DisplayWriteString (char * string, uint8_t start);

/**
 * @brief Write a password to the display
 * @param position Position to write the number (first digit is 0)
 * @param number Number to write
 * @example DisplayWritePassword(2, 5); // Write '--5-' to the display
 */
void DisplayWritePassword (uint8_t position, uint8_t number);

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
int8_t DisplaySetBrightness (int8_t brightness);


/*******************************************************************************
 ******************************************************************************/

#endif // _DISPLAY_H_
