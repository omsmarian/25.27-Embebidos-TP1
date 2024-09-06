/***************************************************************************//**
  @file     pisr.c
  @brief    Periodic Interrupt (PISR) driver
  @author   Group 4
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdlib.h>
#include "pisr.h"
#include "SysTick.h"
#include "hardware.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define DEVELOPMENT_MODE    1


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef struct
{
    pisr_callback_t callback;
    uint32_t period;
    uint32_t counter;
} pisr_t;


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

void pisr (void);


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static pisr_t pisr_callbacks[PISR_CANT];


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

bool pisrRegister (pisr_callback_t fun, unsigned int period)
{
	static bool init_flag = false, status = false;

	if (!init_flag) // Already initialized
	{
		SysTick_Init(pisr, PISR_FREQUENCY_HZ);
		for (uint8_t i = 0; i < PISR_CANT; i++)
			pisr_callbacks[i].callback = NULL;
		init_flag = true;
	}

	for (uint8_t i = 0; i < PISR_CANT; i++)
	{
		if (pisr_callbacks[i].callback != NULL)
		{
			pisr_callbacks[i].callback = fun;
			pisr_callbacks[i].period = pisr_callbacks[i].counter = period;
			status = true;
		}
	}

	return init_flag && status;
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void pisr (void)
{
	for (uint8_t i = 0; i < PISR_CANT; i++)
		if (pisr_callbacks[i].callback && !(--pisr_callbacks[i].counter))
		{
			pisr_callbacks[i].counter = pisr_callbacks[i].period;
			pisr_callbacks[i].callback();
		}
}


/******************************************************************************/
