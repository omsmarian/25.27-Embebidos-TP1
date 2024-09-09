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
#include "encoder.h"

#include "users.h"



void read_console(void);
int read_key(void);
void update_fsl();
void update_menu();
void clear_terminal();
void init_fsl();

void access_system_call(void);

enum states_fsl {ADD_USER, DELETE_USER, CHANGE_PASSWORD, ACCESS_SYSTEM};



#endif /* FSL_H_ */
