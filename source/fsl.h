/*
 * fsl.h
 *
 *  Created on: 2 sept 2024
 *      Author: asolari
 */

#ifndef FSL_H_
#define FSL_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "fsl_debug_console.h"

#include "users.h"

#include "encoder.h"
#include "display.h"
#include "macros.h"
#include "magcard.h"
#include "LEDs.h"
#include "timer.h"
#include "gpio.h"
#include "board.h"



static bool access_flag = false;
static bool error_flag = false;

static ticks_t timer_access;
static ticks_t timer_error;




void read_console(void);
int read_key(void);
void update_fsl();
void update_menu();
void clear_terminal();
void init_fsl();

bool read_from_encoder(char *id);

void access_system_call(void);

enum states_fsl {ADD_USER, DELETE_USER, CHANGE_PASSWORD, ACCESS_SYSTEM, BRIGHTNESS};



#endif /* FSL_H_ */
