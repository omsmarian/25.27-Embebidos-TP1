/***************************************************************************//**
  @file     SysTick.h
  @brief    SysTick driver
  @author   Group 4
 ******************************************************************************/

#ifndef _SYSTICK_H_
#define _SYSTICK_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdbool.h>
#include <stdint.h>


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

// #define SYSTICK_ISR_FREQUENCY_HZ 5000U


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialize SysTic driver
 * @param funcallback Function to be call every SysTick
 * @return Initialization and registration succeed
 */
bool SysTick_Init(void (*funcallback)(void), uint32_t freq);


/*******************************************************************************
 ******************************************************************************/

#endif // _SYSTICK_H_
