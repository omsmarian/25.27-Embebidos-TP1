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


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define DEVELOPMENT_MODE		1

#define PWM_FREQUENCY_HZ		(DISPLAY_DIGITS * DISPLAY_MAX_BRIGHTNESS * DISPLAY_FREQUENCY_HZ) // SysTick minimum frequency is 20kHz!!!

// #define DISPLAY_PERIOD_S	(1 / DISPLAY_REFRESH_HZ)


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

/**
 * @brief Refresh the display
 * @note This function is called by the Periodic Interrupt Service Routine (PISR)
 */
static void RefreshDisplay (void);

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
								 0b00110111,   // A
								 0b01111001,   // B
								 0b00111001,   // C
								 0b01011110,   // D
								 0b01111001,   // E
								 0b01110001,   // F
								 0b01111100,   // G
								 0b00110111,   // H
								 0b00000110,   // I
								 0b00011110,   // J
								 0b01110000,   // K
								 0b00111000,   // L
								 0b00010101,   // M
								 0b00110111,   // N
								 0b00111111,   // O
								 0b01110011,   // P
								 0b01100111,   // Q
								 0b00110001,   // R
								 0b01101101,   // S
								 0b01111000,   // T
								 0b00111110,   // U
								 0b00111110,   // V
								 0b00101010,   // W
								 0b01110110,   // X
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

static uint8_t _buffer[DISPLAY_DIGITS] = { DISPLAY_CLEAR };
// static uint8_t _index = 0;
static uint8_t _brightness = 50;


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

void DisplayWrite			(char* characters)				{ FOR(0, DISPLAY_DIGITS) _buffer[i] = __Char2Segments__(characters[i]); }
void DisplaySetDigit		(uint8_t digit, char character)	{ _buffer[digit] = __Char2Segments__(character); }
void DisplayWriteNumber		(uint16_t number)				{ FOR(0, DISPLAY_DIGITS) _buffer[i] = (number / ((int) pow(10, i))) % 10;}
void DisplayClear			(void)							{ FOR(0, DISPLAY_DIGITS) _buffer[i] = __Char2Segments__(DISPLAY_CLEAR); }
void DisplaySetBrightness	(uint8_t brightness)			{ _brightness = CAP(brightness, DISPLAY_MIN_BRIGHTNESS, DISPLAY_MAX_BRIGHTNESS); }




/*******************************************************************************
 *******************************************************************************
						LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void RefreshDisplay (void)
{
	static uint8_t _index = 0, _count = 0;
	uint8_t character = _characters[_buffer[_index]];

	if ((_count % DISPLAY_MAX_BRIGHTNESS) >  _brightness) // PWM
		character = 0;

	FOR(0, DISPLAY_DIGITS / 2)	gpioWrite(_digits[i], _index & (1 << i)); // Select digit (Encoded)
	FOR(0, DISPLAY_SEGMENTS)	gpioWrite(_segments[i], character & (1 << i)); // Select segments (Write character)

	if ((_count % DISPLAY_MAX_BRIGHTNESS) == 0) // Next digit (Every 4 cycles * 100 steps)
	{
		_index++;
		_index %= DISPLAY_DIGITS;
	}

	_count++;
	_count %= DISPLAY_MAX_BRIGHTNESS * 4;
}

// Helper functions ////////////////////////////////////////////////////////////

uint8_t __Char2Segments__ (char character)
{
	uint8_t buffer = 0;

		 if (character >= '0' && character <= '9') 	{ buffer = character - '0'; }
	else if (character >= 'A' && character <= 'Z')	{ buffer = character - 'A' + 10; }
	else if (character == '-')						{ buffer = 36; } // DASH
	else if (character == '.')						{ buffer = 37; } // DP
	else						  					{ buffer = 38; } // CLEAR

	return buffer;
}


/******************************************************************************/

// Notes ///////////////////////////////////////////////////////////////////////

// The display is refreshed every 1/50 of a second (20ms) to avoid flickering
// Input-output? Char? String? Number?
// _caracters[], etc. location?
// Brightness levels?
// PWM: 1 per 100 steps or 1 per step?
