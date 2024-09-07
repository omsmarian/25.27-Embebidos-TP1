/***************************************************************************//**
  @file     pisr.c
  @brief    Periodic Interrupt (PISR) driver
  @author   Group 4
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "pisr.h"
#include "hardware.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define SYSTICK_LOAD_INIT   ((__CORE_CLOCK__/PISR_FREQUENCY_HZ) - 1U)


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef struct {
	pisr_callback_t funadrrs;
	uint32_t time;
	uint32_t counter;
} funData;


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static funData funArr[PISR_CANT];
static bool init_flag;
static uint32_t count;


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

bool pisrRegister (pisr_callback_t fun, unsigned int period)
{
	if(!init_flag) // initialize the systick
	{
		SysTick->CTRL = 0x00;
		SysTick->LOAD = SYSTICK_LOAD_INIT;
		SysTick->VAL= 0x00;
		SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk|SysTick_CTRL_TICKINT_Msk|SysTick_CTRL_ENABLE_Msk;
		init_flag = true;
	}
	
	if(count < PISR_CANT) // load the funtion and its period
	{
		funArr[count].funadrrs = fun;
		funArr[count].time = period;
		funArr[count++].counter = period;
	}
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

__ISR__ SysTick_Handler(void)
{
	for(int i = 0; i < count ; i++)
		if(--((funArr[i]).counter) == 0)
		{
			(funArr[i]).funadrrs();
			(funArr[i]).counter = (funArr[i]).time;
		}
}


/******************************************************************************/
