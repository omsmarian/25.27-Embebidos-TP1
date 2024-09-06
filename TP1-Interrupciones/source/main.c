/*
 * Copyright 2016-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file    tp1_interrupciones.c
 * @brief   Application entry point.
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "SysTick.h"
#include "fsl.h"




/* TODO: insert other include files here. */

/* TODO: insert other definitions and declarations here. */

/*
 * @brief   Application entry point.
 */
int main(void) {

    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
	BOARD_InitDebugConsole();
	PRINTF("Starting the program\n");

	init_fsl();


	while(1)
	{
		update_fsl();

	}



    return 0 ;
}



















