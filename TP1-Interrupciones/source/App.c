/***************************************************************************//**
  @file     App.c
  @brief    Application functions
  @author   Grupo 4
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "board.h"
#include "encoder.h"
//#include "fsl_debug_console.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

action_t miau;
/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init(void) {
  gpioWrite(PIN_LED_GREEN, HIGH);
  gpioWrite(PIN_LED_BLUE, HIGH);
  gpioWrite(PIN_LED_RED, HIGH);
  gpioMode(PIN_LED_GREEN, OUTPUT);
  gpioMode(PIN_LED_BLUE, OUTPUT);
  gpioMode(PIN_LED_RED, OUTPUT);
  encoder_Init();
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run(void) {
  miau = encoderRead();
  switch(miau)
  {
    case NONE:
      gpioWrite(PIN_LED_GREEN, HIGH);
      gpioWrite(PIN_LED_BLUE, HIGH);
      gpioWrite(PIN_LED_RED, HIGH);
      break;
    case RIGHT:
      gpioWrite(PIN_LED_GREEN, LOW);
      gpioWrite(PIN_LED_BLUE, HIGH);
      gpioWrite(PIN_LED_RED, HIGH);
      break;
    case LEFT:
      gpioWrite(PIN_LED_GREEN, HIGH);
      gpioWrite(PIN_LED_BLUE, LOW);
      gpioWrite(PIN_LED_RED, HIGH);
      break;
    case CLICK:
      gpioWrite(PIN_LED_GREEN, HIGH);
      gpioWrite(PIN_LED_BLUE, HIGH);
      gpioWrite(PIN_LED_RED, LOW);
      break;
    case LONG_CLICK:
      gpioWrite(PIN_LED_GREEN, LOW);
      gpioWrite(PIN_LED_BLUE, LOW);
      gpioWrite(PIN_LED_RED, LOW);
      break;
  }
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


/*******************************************************************************
 ******************************************************************************/
