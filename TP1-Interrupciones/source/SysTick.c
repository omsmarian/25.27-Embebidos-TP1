/***************************************************************************//**
  @file     SysTick.c
  @brief    SysTick driver
  @author   Group 4
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "SysTick.h"
#include "hardware.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define DEVELOPMENT_MODE		1

#define SYSTICK_LOAD_INIT(freq) ((__CORE_CLOCK__/freq) - 1U)


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void (*systick_callback)(void);


/*******************************************************************************
 *******************************************************************************
						GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

bool SysTick_Init(void (*funcallback)(void), uint32_t freq)
{
	// SysTick_Config(SYSTICK_LOAD_INIT);
	static bool yaInit = false;
#if DEVELOPMENT_MODE
	if (!yaInit && funcallback && SYSTICK_LOAD_INIT(freq) < SysTick_LOAD_RELOAD_Msk)
#endif // DEVELOPMENT_MODE
	{
		SysTick->CTRL = 0x00;
		SysTick->LOAD = SYSTICK_LOAD_INIT(freq);
		// NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
		SysTick->VAL  = 0x00;
		SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;

		systick_callback = funcallback;
		yaInit = true;
	}
	return yaInit;
}


/*******************************************************************************
 *******************************************************************************
						LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

__ISR__ SysTick_Handler(void)
{
#if DEVELOPMENT_MODE
	if (systick_callback)
#endif // DEVELOPMENT_MODE
	{
		systick_callback();
	}
}


/******************************************************************************/
