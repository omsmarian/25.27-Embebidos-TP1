/***************************************************************************//**
  @file     encoder.h
  @brief    Encoder driver
  @author   Group 4
 ******************************************************************************/

#ifndef _ENCODER_H_
#define _ENCODER_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdbool.h>
#include "gpio.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/***** ENCODER defines ********************************************************/

//// Encoder
//#define PIN_ENCODER_RCHA		PORTNUM2PIN(PB, 9)	// PTC12
//#define PIN_ENCODER_RCHB		PORTNUM2PIN(PA, 1)	// PTC4
//#define PIN_ENCODER_RSWITCH		PORTNUM2PIN(PB, 23) // PTD0
// Encoder
#define PIN_ENCODER_RCHA		PORTNUM2PIN(PE, 24)	// PTC12
#define PIN_ENCODER_RCHB		PORTNUM2PIN(PE, 25)	// PTC4
#define PIN_ENCODER_RSWITCH		PORTNUM2PIN(PE, 26) // PTD0

// LEDs
#define PIN_LED_RED				PORTNUM2PIN(PB, 22)	// PTB22
#define PIN_LED_GREEN			PORTNUM2PIN(PE, 26)	// PTE26
#define PIN_LED_BLUE			PORTNUM2PIN(PB, 21)	// PTB21


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum {
	NONE,
	RIGHT,
	LEFT,
	CLICK,
	LONG_CLICK,
	DOUBLE_CLICK
} action_t;


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialize the encoder
 * @return return true if the encoder was initialized successfully
 */
bool encoder_Init(void);

/**
 * @brief Reads the encoder gesture
 * @return return the gesture read from the encoder
 */
action_t encoderRead(void);

/**
 * @brief Sets the enconder to default settings
 */
void ResetEncoder(void);


/*******************************************************************************
 ******************************************************************************/

#endif // _ENCODER_H_
