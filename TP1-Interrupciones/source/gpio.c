/*
 * gpio.c
 *
 *  Created on: 12 ago. 2024
 *      Author: Valentin U. Vieira
 */

#include <stdio.h>
#include "gpio.h"
#include "board.h"
#include "hardware.h"
#include "MK64F12.h"

static GPIO_Type * const GPIO_Ports[] = GPIO_BASE_PTRS;
static PORT_Type * const PORT_Ports[] = PORT_BASE_PTRS;
static uint32_t SCSCGC5Ports_t[] = 	{ SIM_SCGC5_PORTA(HIGH), SIM_SCGC5_PORTB(HIGH),
									  SIM_SCGC5_PORTC(HIGH), SIM_SCGC5_PORTD(HIGH),
									  SIM_SCGC5_PORTE(HIGH) };

void gpioMode (pin_t pin, uint8_t mode)
{
//	SIM->SCGC5 &= ~SIM_SCGC5_PORTB(HIGH);
	SIM->SCGC5 |= SCSCGC5Ports_t[PIN2PORT(pin)];
	PORT_Ports[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] |= PORT_PCR_MUX(PORT_mGPIO);

	if (mode == OUTPUT)
		GPIO_Ports[PIN2PORT(pin)]->PDDR |= (HIGH<<(PIN2NUM(pin)));
	else
	{
		GPIO_Ports[PIN2PORT(pin)]->PDDR |= (LOW<<(PIN2NUM(pin)));

		if (mode != INPUT)
		{
			PORT_Ports[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] |= PORT_PCR_PE(HIGH);

			if (mode == INPUT_PULLUP)
				PORT_Ports[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] |= PORT_PCR_PS(HIGH);
			else
				PORT_Ports[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] |= PORT_PCR_PS(LOW);
		}
		else
			PORT_Ports[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] |= PORT_PCR_PE(LOW);
	}
}

void gpioWrite (pin_t pin, bool value)
{
//	GPIO_Ports[PIN2PORT(pin)]->PDOR &= ~(HIGH<<PIN2NUM(pin)); // clear bit
//	GPIO_Ports[PIN2PORT(pin)]->PDOR |= (value<<PIN2NUM(pin)); // change bit
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
