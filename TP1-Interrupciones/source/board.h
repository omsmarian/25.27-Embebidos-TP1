/***************************************************************************//**
  @file     board.h
  @brief    Board management
  @author   Group 4
 ******************************************************************************/

#ifndef _BOARD_H_
#define _BOARD_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "gpio.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/***** BOARD defines **********************************************************/

// On Board User LEDs
#define PIN_LED_RED				PORTNUM2PIN(PB, 22)	// PTB22
#define PIN_LED_GREEN			PORTNUM2PIN(PE, 26)	// PTE26
#define PIN_LED_BLUE			PORTNUM2PIN(PB, 21)	// PTB21

#define LED_ACTIVE				LOW


// On Board User Switches
#define PIN_SW2					PORTNUM2PIN(PC, 6)	// PTC6
#define PIN_SW3					PORTNUM2PIN(PA, 4)	// PTA4

#define SW_ACTIVE				LOW
#define SW_INPUT_TYPE			INPUT_PULLUP


// External User LEDs
#define PIN_LED_EXT_STATUS0		PORTNUM2PIN(PA, 0)	// PTA0
#define PIN_LED_EXT_STATUS1		PORTNUM2PIN(PA, 0)	// PTA0


// Encoder
#define PIN_ENCODER_RCHA		PORTNUM2PIN(PA, 0)	// PTA0
#define PIN_ENCODER_RCHB		PORTNUM2PIN(PA, 0)	// PTA0
#define PIN_ENCODER_RSWITCH		PORTNUM2PIN(PA, 0)	// PTA0


// Display
#define PIN_DISPLAY_CSEGA		PORTNUM2PIN(PA, 0)	// PTA0
#define PIN_DISPLAY_CSEGB		PORTNUM2PIN(PA, 0) 	// PTA0
#define PIN_DISPLAY_CSEGC		PORTNUM2PIN(PA, 0) 	// PTA0
#define PIN_DISPLAY_CSEGD		PORTNUM2PIN(PA, 0)	// PTA0
#define PIN_DISPLAY_CSEGE		PORTNUM2PIN(PA, 0)	// PTA0
#define PIN_DISPLAY_CSEGF		PORTNUM2PIN(PA, 0) 	// PTA0
#define PIN_DISPLAY_CSEGG		PORTNUM2PIN(PA, 0) 	// PTA0
#define PIN_DISPLAY_CSEGDP		PORTNUM2PIN(PA, 0) 	// PTA0
#define PIN_DISPLAY_SEL0		PORTNUM2PIN(PA, 0) 	// PTA0
#define PIN_DISPLAY_SEL1		PORTNUM2PIN(PA, 0) 	// PTA0


// Magnetic Stripe Card Reader
#define PIN_MAGCARD_DATA		PORTNUM2PIN(PB, 23)	// PTB23
#define PIN_MAGCARD_CLOCK		PORTNUM2PIN(PA, 1)	// PTA1
#define PIN_MAGCARD_ENABLE		PORTNUM2PIN(PB, 9)	// PTB9


/*******************************************************************************
 ******************************************************************************/

#endif // _BOARD_H_
