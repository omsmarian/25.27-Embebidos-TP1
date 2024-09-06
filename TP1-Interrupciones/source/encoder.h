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

#include "hardware.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/***** ENCODER defines **********************************************************/


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
