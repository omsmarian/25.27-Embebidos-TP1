/***************************************************************************//**
  @file     App.c
  @brief    Application functions
  @author   Group 4
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "hardware.h"
#include "board.h"
#include "gpio.h"
#include "SysTick.h"
#include "display.h"
#include "encoder.h"
#include "magcard.h"
#include "pisr.h"
//#include "fsl_debug_console.h"


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

void fun (void);


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

MagCard_t magCard;


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init(void) {
//  printf("%d", MagCardInit());
//	MagCardInit();
	DisplayInit();
	pisrRegister(fun, PISR_FREQUENCY_HZ / 10);
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run(void) {
//  printf("%d", MagCardCheckData());
//  printf("%d", MagCardGetData());
//	if(MagCardGetStatus())
//	{
//		magCard = *MagCardGetData();
//		gpioWrite(PIN_LED_BLUE, LOW);
//		gpioWrite(PIN_LED_RED, HIGH);
//	}
//	char characters[4] = { '1', '2', '3', '4' };
//	DisplayClear();
//	LEDS_On(NO_LED);
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void fun (void)
{
	static uint8_t i = 0;
	char characters[4] = { 1, 2, 3, 4 };
	DisplaySetBrightness(i++);
	DisplayWrite(characters);
	i %= MAX_BRIGHTNESS;
}


/*******************************************************************************
 ******************************************************************************/
