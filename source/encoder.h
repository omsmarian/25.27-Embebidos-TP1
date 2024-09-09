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
