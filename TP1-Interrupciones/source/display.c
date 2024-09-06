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

#define DEVELOPMENT_MODE		1

// #define DISPLAY_PERIOD_S	(1 / DISPLAY_REFRESH_HZ)

#define CAP(x, min, max)		(x < (min) ? (min) : (x > (max) ? (max) : x))


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void RefreshDisplay (void);


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static const uint8_t characters [11] = { 0b00111111,
										 0b00000110,
										 0b01011011,
										 0b01001111,
										 0b01100110,
										 0b01101101,
										 0b01111101,
										 0b00000111,
										 0b01111111,
										 0b01101111,
										 0b00000000 };

// static const uint8_t select [4] = { 0x00, 0x01, 0x02, 0x03 };

static uint8_t display_buffer[DISPLAY_LENGTH] = { 10 };
// static uint8_t display_index = 0;

static uint8_t display_brightness = 100;


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
		gpioMode(PIN_DISPLAY_CSEGF,		OUTPUT);
		gpioMode(PIN_DISPLAY_CSEGG,		OUTPUT);
		gpioMode(PIN_DISPLAY_CSEGDP,    OUTPUT);
		gpioMode(PIN_DISPLAY_SEL0,		OUTPUT);
		gpioMode(PIN_DISPLAY_SEL1,		OUTPUT);

		pisrRegister(RefreshDisplay,  PISR_FREQUENCY_HZ / (DISPLAY_LENGTH * (MAX_BRIGHTNESS) * DISPLAY_FREQUENCY_HZ)); // SysTick minimum frequency is 5000Hz!!!

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
		display_buffer[i] = 10;
}

void DisplaySetBrightness (uint8_t brightness)
{
	display_brightness = CAP(brightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
}


/*******************************************************************************
 *******************************************************************************
						LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void RefreshDisplay (void)
{
	static uint8_t display = 0, count = 0;
	uint8_t character = characters[display_buffer[display]];

	if ((count % MAX_BRIGHTNESS) >  display_brightness)
		character = 0;

//	 gpioWrite(PIN_DISPLAY_CSEGA,  0);
//	 gpioWrite(PIN_DISPLAY_CSEGB,  0);
//	 gpioWrite(PIN_DISPLAY_CSEGC,  0);
//	 gpioWrite(PIN_DISPLAY_CSEGD,  0);
//	 gpioWrite(PIN_DISPLAY_CSEGE,  0);
//	 gpioWrite(PIN_DISPLAY_CSEGF,  0);
//	 gpioWrite(PIN_DISPLAY_CSEGG,  0);
//	 gpioWrite(PIN_DISPLAY_CSEGDP, 0);

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

    if ((count % MAX_BRIGHTNESS) == 0)
    {
    	display++;
    	display %= DISPLAY_LENGTH;
    }
	count++;
	count %= MAX_BRIGHTNESS * 4;
}



// #include "pwm.h"

// void DisplayRefresh(void)
// {
// 	static uint8_t display = 0, count = 0;
// 	uint8_t character = characters[display_buffer[display]];

// 	if (count > display_brightness)
// 		character = 0;

// 	gpioWrite(PIN_DISPLAY_SEL0, display & 0x01);
// 	gpioWrite(PIN_DISPLAY_SEL1, display & 0x02);

// 	gpioWrite(PIN_DISPLAY_CSEGA, character & 0x01);
// 	gpioWrite(PIN_DISPLAY_CSEGB, character & 0x02);
// 	gpioWrite(PIN_DISPLAY_CSEGC, character & 0x04);
// 	gpioWrite(PIN_DISPLAY_CSEGD, character & 0x08);
// 	gpioWrite(PIN_DISPLAY_CSEGE, character & 0x10);
// 	gpioWrite(PIN_DISPLAY_CSEGF, character & 0x20);
// 	gpioWrite(PIN_DISPLAY_CSEGG, character & 0x40);
// 	gpioWrite(PIN_DISPLAY_CSEGDP, character & 0x80);

// 	display++;
// 	display %= 4;
// 	count++;
// }

// bool DisplayInit(void)
// {
// 	static bool init_flag = false;

// 	if (!init_flag) // Already initialized
// 	{
// 		gpioMode(PIN_DISPLAY_CSEGA, OUTPUT);
// 		gpioMode(PIN_DISPLAY_CSEGB, OUTPUT);
// 		gpioMode(PIN_DISPLAY_CSEGC, OUTPUT);
// 		gpioMode(PIN_DISPLAY_CSEGD, OUTPUT);
// 		gpioMode(PIN_DISPLAY_CSEGE, OUTPUT);
// 		gpioMode(PIN_DISPLAY_CSEGF, OUTPUT);
// 		gpioMode(PIN_DISPLAY_CSEGG, OUTPUT);
// 		gpioMode(PIN_DISPLAY_CSEGDP, OUTPUT);
// 		gpioMode(PIN_DISPLAY_SEL0, OUTPUT);
// 		gpioMode(PIN_DISPLAY_SEL1, OUTPUT);

// 		pwmInit(PWM_TIMER, PWM_CHANNEL, PWM_FREQUENCY_HZ);
// 		pwmWrite(PWM_TIMER, PWM_CHANNEL, display_brightness);

// 		init_flag = true;
// 	}

// 	return init_flag;
// }

// void DisplaySetBrightness(uint8_t brightness)
// {
// 	display_brightness = CAP(brightness, 0, 100);
// 	pwmWrite(PWM_TIMER, PWM_CHANNEL, display_brightness);
// }


/******************************************************************************/
