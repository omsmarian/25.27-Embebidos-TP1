/***************************************************************************//**
  @file     display.c
  @brief    Display driver
  @author   Group 4
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <math.h>
#include "display.h"
#include "board.h"
#include "gpio.h"
#include "pisr.h"
#include "macros.h"


#include "fsl_debug_console.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define DEVELOPMENT_MODE		1

#define PWM_FREQUENCY_HZ		(DISPLAY_DIGITS * DISPLAY_MAX_BRIGHTNESS * DISPLAY_FREQUENCY_HZ) // SysTick minimum frequency is 20kHz!!!
// #define DISPLAY_PERIOD_S	(1 / DISPLAY_REFRESH_HZ)

#define NUMBERS					10
#define LETTERS					26

#define DASH					NUMBERS + LETTERS
#define DP						NUMBERS + LETTERS + 1
#define CLEAR					NUMBERS + LETTERS + 2


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

/**
 * @brief Refresh the display
 * @note This function is called by the Periodic Interrupt Service Routine (PISR)
 */
static void RefreshDisplay (void);

// Helper functions ////////////////////////////////////////////////////////////

/**
 * @brief Convert a character to its corresponding segments
 * @param character Character to convert
 * @return Corresponding segments
 * @note If the character is not valid, CLEAR is returned
 * @example __Char2Segments__('A'); // Returns 0b00110111
 */
uint8_t __Char2Segments__ (char character);


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

const uint8_t _characters [] = { 0b00111111,   // 0
								 0b00000110,   // 1
								 0b01011011,   // 2
								 0b01001111,   // 3
								 0b01100110,   // 4
								 0b01101101,   // 5
								 0b01111101,   // 6
								 0b00000111,   // 7
								 0b01111111,   // 8
								 0b01101111,   // 9
								 0b01110111,   // A
								 0b01111100,   // b
								 0b00111001,   // C
								 0b01011110,   // d
								 0b01111001,   // E
								 0b01110001,   // F
								 0b01111101,   // G
								 0b01110110,   // H
								 0b00110000,   // I
								 0b00001110,   // J
								 0b00000000,   // K?
								 0b00111000,   // L
								 0b00000000,   // M?
								 0b01010100,   // n
								 0b01011100,   // o
								 0b01110011,   // P
								 0b01100111,   // Q
								 0b01010000,   // r
								 0b01101101,   // S
								 0b01111000,   // t
								 0b00111110,   // U
								 0b00000000,   // V?
								 0b00000000,   // W?
								 0b00000000,   // X?
								 0b01101110,   // Y
								 0b01011011,   // Z
								 0b01000000,   // DASH
								 0b10000000,   // DP
								 0b00000000 }; // CLEAR

const uint8_t _segments [] = { PIN_DISPLAY_CSEGA,
							   PIN_DISPLAY_CSEGB,
							   PIN_DISPLAY_CSEGC,
							   PIN_DISPLAY_CSEGD,
							   PIN_DISPLAY_CSEGE,
							   PIN_DISPLAY_CSEGF,
							   PIN_DISPLAY_CSEGG,
							   PIN_DISPLAY_CSEGDP };

const uint8_t _digits [] = { PIN_DISPLAY_SEL1,
						     PIN_DISPLAY_SEL0 };

static uint8_t _buffer[DISPLAY_DIGITS];
// static uint8_t _index = 0;
static int8_t _brightness = DISPLAY_MAX_BRIGHTNESS;
static uint16_t _counterSlide = 0;


/*******************************************************************************
 *******************************************************************************
						GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

bool DisplayInit (void)
{
	static bool _init = false;

	if (!_init && (_init = true)) // Only initialize once
	{
		FOR(0, DISPLAY_SEGMENTS)	gpioMode(_segments[i], OUTPUT);
		FOR(0, DISPLAY_DIGITS / 2)	gpioMode(_digits[i], OUTPUT);

		pisrRegister(RefreshDisplay,  PISR_FREQUENCY_HZ / PWM_FREQUENCY_HZ);
	}

	return _init;
}

void DisplaySetDigitChar (uint8_t digit, char character)
{
	if (digit < DISPLAY_DIGITS)
		_buffer[digit] = __Char2Segments__(character);
}

void DisplaySetDigitNum (uint8_t digit, uint8_t number)
{
	if (digit < DISPLAY_DIGITS && number < NUMBERS)
		_buffer[digit] = number;
}

void DisplayClearDigit (uint8_t digit)
{
	if (digit < DISPLAY_DIGITS)
		_buffer[digit] = CLEAR;
}

void DisplayWriteChar (char * characters)
{
	DisplayClear();

	for (uint8_t i = 0; i < DISPLAY_DIGITS && characters[i] != '\0'; i++)
		_buffer[i] = __Char2Segments__(characters[i]);
}

void DisplayWriteNum (uint8_t n1, uint8_t n2, uint8_t n3, uint8_t n4)
{
	DisplayClear();

	if (n1 > NUMBERS) _buffer[0] = n1;
	if (n2 > NUMBERS) _buffer[1] = n2;
	if (n3 > NUMBERS) _buffer[2] = n3;
	if (n4 > NUMBERS) _buffer[3] = n4;
}

void DisplayWriteString (char * string, uint8_t start)
{
	DisplayClear();

	uint8_t length = 0;
	while (string[length++] != '\0')

	CAP(start, 0, length - DISPLAY_DIGITS);
	DisplayWriteChar(&string[start]);
}

void DisplayWritePassword(uint8_t position, uint8_t number)
{
	//DisplayClear();

	char str[] = "----";
	position = CAP(position, 0, (DISPLAY_DIGITS - 1));
	str[position] = number + '0';

	DisplayWriteChar(str);
}

void DisplayClear (void)
{
	for (uint8_t i = 0; i < DISPLAY_DIGITS; i++)
		_buffer[i] = CLEAR;
}

int8_t DisplaySetBrightness (int8_t brightness)
{
	_brightness +=  brightness;
	_brightness = CAP(_brightness, DISPLAY_MIN_BRIGHTNESS, DISPLAY_MAX_BRIGHTNESS);
	return _brightness;
}


/*******************************************************************************
 *******************************************************************************
						LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void RefreshDisplay (void)
{
	static uint8_t _index = 0, _count = 0;
	uint8_t character = _characters[_buffer[_index]];

	if (!(++_counterSlide % 100))
	{
		_counterSlide = 0;
		_index++;
		if(_buffer[_index]=='\0')
			_index = 0;
	}

	if (((_count + 1) % DISPLAY_MAX_BRIGHTNESS) >=  _brightness) // PWM
		character = 0;

	FOR(0, DISPLAY_SEGMENTS)	gpioWrite(_segments[i], character & (0 << i));
	FOR(0, DISPLAY_DIGITS / 2)	gpioWrite(_digits[i], _index & (1 << i)); // Select digit (Encoded)
	FOR(0, DISPLAY_SEGMENTS)	gpioWrite(_segments[i], character & (1 << i)); // Select segments (Write character)

	if (((_count + 1) % DISPLAY_MAX_BRIGHTNESS) == 0) // Next digit (Every 4 cycles * 100 steps)
	{
		_index++;
		_index %= DISPLAY_DIGITS;
	}

	_count++;
	_count %= DISPLAY_MAX_BRIGHTNESS * 4;
}

// Helper functions ////////////////////////////////////////////////////////////

uint8_t __Char2Segments__ (char c)
{
	uint8_t buffer = 0;

		 if (c >= '0' && c <= '9')	{ buffer = c - '0'; }
	else if (c >= 'A' && c <= 'Z')	{ buffer = c - 'A' + NUMBERS; }
	else if (c >= 'a' && c <= 'z')	{ buffer = c - 'a' + NUMBERS; }
	else if (c == '-')				{ buffer = DASH; } // DASH
	else if (c == '.')				{ buffer = DP; } // DP
	else						  	{ buffer = CLEAR; } // CLEAR

	return buffer;
}


/******************************************************************************/
