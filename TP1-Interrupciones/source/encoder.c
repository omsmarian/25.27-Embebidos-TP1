/***************************************************************************//**
  @file     encoder.c
  @brief    Encoder driver
  @author   Grupo 4
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "encoder.h"
#include "board.h"
#include "timer.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define DEVELOPMENT_MODE    1


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

// typedef enum
// {
//     None,
// 	RCHA,
// 	RCHB,
// } EncoderNegEdge_t;

// typedef enum
// {
//     None,
// 	Right,
// 	Left,
// } EncoderTurnDir_t;


/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

// /**
//  * @brief Periodic service
//  */
// static void encoder_isr(void);


/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

// static bool flag_switch = 0;
// static bool flag_turn_right = 0;
// static bool flag_turn_left = 0;

// static EncoderNegEdge_t flag_first = None;
// static EncoderTurnDir_t flag_turn = None;


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

bool encoder_Init(void) {
    gpioMode(PIN_ENCODER_RCHA, INPUT_PULLUP);
    gpioMode(PIN_ENCODER_RCHB, INPUT_PULLUP);
    gpioMode(PIN_ENCODER_RSWITCH, INPUT_PULLUP);

    timerInit();
    timerStart(timerGetId(), TIMER_MS2TICKS(10), TIM_MODE_PERIODIC, encoder_isr);

    return 0;
}

EncoderTurnDir_t encoderRead(void) {
    return flag_turn;
}

void encoderReset(void) {
    flag_switch = 0;
    flag_turn_right = 0;
    flag_turn_left = 0;
}

bool encoderSet(void) {
    return 0;
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

// static void encoder_isr(void) {
//     if(gpioRead(PIN_ENCODER_RSWITCH) == 0) { // Falta el debounce
//         flag_switch = 1;
//     }
//     // else {
//     //     flag_switch = 0;
//     // }
//     switch (flag_first)
//     {
//     case None:
//         flag_turn_right = 0;
//         flag_turn_left = 0;
//         break;

//     case RCHA:
//         if (gpioRead(PIN_ENCODER_RCHB))
//             flag_turn_right = 1;
//             flag_turn_left = 0;
//             flag_first = NONE;
//         break;

//     case RCHB:
//         if (gpioRead(PIN_ENCODER_RCHA))
//             flag_turn_right = 0;
//             flag_turn_left = 1;
//             flag_first = NONE;
//         break;
//     }
// }


/******************************************************************************/
