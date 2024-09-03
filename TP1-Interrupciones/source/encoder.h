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
#include "hardware.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/***** ENCODER defines **********************************************************/


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

enum {
  RIGHT,
  LEFT,
  CLICK,
  LONG_CLICK,};

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
uint8_t encoderRead(void);

// /**
//  * @brief
//  * @param
//  * @return
//  */
// bool encoderGetCurrentPos(void);

// /**
//  * @brief
//  * @param
//  * @return
//  */
// void encoderReset(void);

// /**
//  * @brief
//  * @param
//  * @return
//  */
// bool encoderSet(void);


/*******************************************************************************
 ******************************************************************************/

#endif // _ENCODER_H_
