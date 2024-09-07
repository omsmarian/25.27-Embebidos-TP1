/***************************************************************************//**
  @file     gpio.h
  @brief    Simple GPIO Pin services, similar to Arduino
  @author   Group 4
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdio.h>
#include "gpio.h"
#include "board.h"
#include "hardware.h"
#include "MK64F12.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define BITGET(x,n) 		((x) & (1<<(n)))
#define BITSET(x,n) 		((x) |= (1<<(n)))
#define BITCLR(x,n) 		((x) &= ~(1<<(n)))
#define BITTOG(x,n) 		((x) ^= (1<<(n)))
#define BITINV(x,n) 		((x) = (x) ^ (1<<(n)))
#define BITVAL(x,n) 		((x) >> (n) & 1)
#define BITMSK(x,n) 		((x) & ~(1<<(n)))
#define BITSETVAL(x,n,v)	((x) = (x) ^ ((-(v) ^ (x)) & (1 << (n))))
#define BITCLVAL(x,n,v) 	((x) = (x) & ~(1 << (n)) | ((v) << (n)))
#define BITTOGVAL(x,n,v) 	((x) = (x) ^ ((v) << (n)))
#define BITINVVAL(x,n,v) 	((x) = (x) ^ ((v) << (n)))
#define BITMSKVAL(x,n,v) 	((x) = (x) & ~(1 << (n)))
#define BITRNG(x,m,n) 		((x) & (((1 << ((m) - (n) + 1)) - 1) << (n)))
#define BITRNGVAL(x,m,n,v) 	((x) = (x) & ~(((1 << ((m) - (n) + 1) - 1) << (n)) | ((v) << (n))))
#define BITCPY(x,m,n,y) 	((x) = (x) & ~(((1 << ((m) - (n) + 1) - 1) << (n)) | ((y) & ((1 << ((m) - (n) + 1) - 1)) << (n))))
#define BITCPYVAL(x,m,n,y) 	((x) = (x) & ~(((1 << ((m) - (n) + 1) - 1) << (n)) | ((y) << (n))))
#define BITINS(x,m,n,y) 	((x) = (x) & ~(((1 << ((m) - (n) + 1) - 1) << (n)) | ((y) << (n))))
#define BITEXT(x,m,n) 		((x) >> (n) & ((1 << ((m) - (n) + 1)) - 1))
#define BITEXTVAL(x,m,n,v) 	((x) = (x) & ~(((1 << ((m) - (n) + 1) - 1) << (n)) | ((v) << (n))))
#define BITCNT(x) 			((x) & 1 + BITCNT((x) >> 1))
#define BITREV(x) 			((x) = (((x) & 0x55555555) << 1) | (((x) & 0xAAAAAAAA) >> 1))
#define BITSWP2(x) 			((x) = ((x) & 0x33333333) << 2 | ((x) & 0xCCCCCCCC) >> 2)
#define BITSWP4(x) 			((x) = ((x) & 0x0F0F0F0F) << 4 | ((x) & 0xF0F0F0F0) >> 4)
#define BITSWP8(x) 			((x) = ((x) & 0x00FF00FF) << 8 | ((x) & 0xFF00FF00) >> 8)
#define BITSWP16(x) 		((x) = ((x) & 0x0000FFFF) << 16 | ((x) & 0xFFFF0000) >> 16)
#define BITPAR(x) 			((x) = ((x) & 0x55555555) + (((x) >> 1) & 0x55555555), \
		   					 (x) = ((x) & 0x33333333) + (((x) >> 2) & 0x33333333), \
							 (x) = ((x) & 0x0F0F0F0F) + (((x) >> 4) & 0x0F0F0F0F), \
							 (x) = ((x) & 0x00FF00FF) + (((x) >> 8) & 0x00FF00FF), \
							 (x) = ((x) & 0x0000FFFF) + (((x) >> 16) & 0x0000FFFF))


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

pinIrqFun_t irqFuns[32] = { NULL };


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

uint8_t PinBit2Num(uint32_t pin);


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static GPIO_Type * const    GPIO_Ports[]        =   GPIO_BASE_PTRS;
static PORT_Type * const    PORT_Ports[]        =   PORT_BASE_PTRS;
static uint32_t             SCSCGC5Ports_t[]    = { SIM_SCGC5_PORTA(HIGH),
									  	  	        SIM_SCGC5_PORTB(HIGH),
											        SIM_SCGC5_PORTC(HIGH),
											        SIM_SCGC5_PORTD(HIGH),
											        SIM_SCGC5_PORTE(HIGH) };
static uint8_t const        PORT_IRQn[] 		=   PORT_IRQS;
static uint8_t const        GPIO_IRQn[] 		= { PORT_eDisabled,
									 	 	        PORT_eInterruptRising,
											        PORT_eInterruptFalling,
											        PORT_eInterruptEither };


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void gpioMode (pin_t pin, uint8_t mode)
{
	PINData_t pin_data = { PIN2PORT(pin), PIN2NUM(pin) };
	uint32_t * pin_PCR = &(PORT_Ports[pin_data.port]->PCR[pin_data.num]);
	uint32_t * pin_PDDR = &(GPIO_Ports[pin_data.port]->PDDR);

	SIM->SCGC5 |= SCSCGC5Ports_t[pin_data.port];
	PORT_Ports[PIN2PORT(pin)]->PCR[pin_data.num] |= PORT_PCR_MUX(PORT_mGPIO);

	*pin_PCR |= PORT_PCR_MUX(PORT_mGPIO);

	if (mode == OUTPUT)
		*pin_PDDR |= (HIGH<<(pin_data.num));
	else
	{
		*pin_PDDR |= (LOW<<(pin_data.num));

		if (mode != INPUT)
		{
			*pin_PCR |= PORT_PCR_PE(HIGH);

			if (mode == INPUT_PULLUP)
				*pin_PCR |= PORT_PCR_PS(HIGH);
			else
				*pin_PCR |= PORT_PCR_PS(LOW);
		}
		else
			*pin_PCR |= PORT_PCR_PE(LOW);
	}
}

bool gpioIRQ (pin_t pin, uint8_t irqMode, pinIrqFun_t irqFun)
{
	NVIC_EnableIRQ(PORT_IRQn[PIN2PORT(pin)]);
	PORT_Ports[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] |= PORT_PCR_IRQC(GPIO_IRQn[irqMode]);
	irqFuns[PIN2NUM(pin)] = irqFun;

	return 0;
}

void gpioWrite (pin_t pin, bool value)
{
//	GPIO_Ports[PIN2PORT(pin)]->PDOR &= ~(HIGH<<PIN2NUM(pin)); // Clear bit
//	GPIO_Ports[PIN2PORT(pin)]->PDOR |= (value<<PIN2NUM(pin)); // Change bit
	if (value)
		GPIO_Ports[PIN2PORT(pin)]->PSOR = (HIGH<<PIN2NUM(pin));
	else
		GPIO_Ports[PIN2PORT(pin)]->PCOR = (HIGH<<PIN2NUM(pin));
}

void gpioToggle (pin_t pin)
{
//	GPIO_Ports[PIN2PORT(pin)]->PDOR ^= (HIGH<<PIN2NUM(pin));
	GPIO_Ports[PIN2PORT(pin)]->PTOR = (HIGH<<PIN2NUM(pin));
}

bool gpioRead (pin_t pin)
{
//	return ((GPIO_Ports[PIN2PORT(pin)]->PDIR & (HIGH<<PIN2NUM(pin)))>>PIN2NUM(pin));
	return GPIO_Ports[PIN2PORT(pin)]->PDIR & (HIGH<<PIN2NUM(pin));
}

__ISR__ PORTA_IRQHandler (void)
{
	// uint8_t pin = PinBit2Num(PORTA->ISFR);
	for (uint8_t i = 0; i < 32; i++)
	{
		if (BITGET(PORTA->ISFR, i))
		{
			PORTA->PCR[i] |= PORT_PCR_ISF(HIGH);
			if(irqFuns[i] != NULL)
				irqFuns[i]();
		}
	}
	// PORTA->PCR[pin] |= PORT_PCR_ISF(HIGH);
	// PinBit2Num(PORTA->ISFR);
	// PinBit2Num(NVIC_GetActive(PORTA));
	// if(irqFuns[pin] != NULL)
		// irqFuns[pin]();
}

__ISR__ PORTB_IRQHandler (void)
{
	uint8_t pin = PinBit2Num(PORTB->ISFR);
	PORTB->PCR[pin] |= PORT_PCR_ISF(HIGH);
	if(irqFuns[pin] != NULL)
		irqFuns[pin]();
}

__ISR__ PORTC_IRQHandler (void)
{
	uint8_t pin = PinBit2Num(PORTC->ISFR);
	PORTC->PCR[pin] |= PORT_PCR_ISF(HIGH);
	if(irqFuns[pin] != NULL)
		irqFuns[pin]();
}

__ISR__ PORTD_IRQHandler (void)
{
	uint8_t pin = PinBit2Num(PORTD->ISFR);
	PORTD->PCR[pin] |= PORT_PCR_ISF(HIGH);
	if(irqFuns[pin] != NULL)
		irqFuns[pin]();
}

__ISR__ PORTE_IRQHandler (void)
{
	uint8_t pin = PinBit2Num(PORTE->ISFR);
	PORTE->PCR[pin] |= PORT_PCR_ISF(HIGH);
	if(irqFuns[pin] != NULL)
		irqFuns[pin]();
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

uint8_t PinBit2Num(uint32_t pin)
{
	uint8_t i = 0;
	while (pin >>= 1)
		i++;

	return i;
}