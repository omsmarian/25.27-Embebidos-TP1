/***************************************************************************//**
  @file     encoder.h
  @brief    Encoder driver
  @author   Grupo 4
 ******************************************************************************/

#ifndef _ENCODER_H_
#define _ENCODER_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include "hardware.h"
#include "gpio.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/***** ENCODER defines **********************************************************/
// Encoder
#define PIN_ENCODER_RCHA      PORTNUM2PIN(PC, 12) // PTC12
#define PIN_ENCODER_RCHB      PORTNUM2PIN(PC, 4) // PTC4
#define PIN_ENCODER_RSWITCH   PORTNUM2PIN(PD, 0) // PTD0

#define PIN_LED_RED     PORTNUM2PIN(PB, 22) // PTB22
#define PIN_LED_GREEN   PORTNUM2PIN(PE, 26) // PTE26
#define PIN_LED_BLUE    PORTNUM2PIN(PB, 21) // PTB21


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum {
  RIGHT,
  LEFT,
  CLICK,
  LONG_CLICK}action_t;

typedef struct{
  action_t direction;
  int32_t action_counter;
}encoder_data_t;
/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

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
encoder_data_t encoderRead(void);

/*******************************************************************************
 ******************************************************************************/

#endif // _ENCODER_H_
