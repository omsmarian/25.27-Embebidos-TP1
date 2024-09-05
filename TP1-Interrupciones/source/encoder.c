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
#include "pisr.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define DEVELOPMENT_MODE    1


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef struct{
  bool RCHA;
  bool RCHB;
  bool RSWITCH;
}state_flags_t;

/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

/**
* @brief Checks for encoder gesture
*/
static void encoderCallback(void);

/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

action_t direction;
bool RCHA;
bool RCHB;
bool RSWITCH;
bool falling_edge;

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

bool encoder_Init(void)
{
  gpioMode(PIN_ENCODER_RCHA, INPUT_PULLUP);
  gpioMode(PIN_ENCODER_RCHB, INPUT_PULLUP);
  gpioMode(PIN_ENCODER_RSWITCH, INPUT_PULLUP);

  pisrRegister(encoderCallback, 1);

  return 0;
}


action_t encoderRead(void)
{

}
/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
/**********************************************
* La joda de este padazo de codigo es q cada
* vez q se llama guarda el estado de los pines
**********************************************/
static void encoderCallback(void)
{
  RCHA = gpioRead(PIN_ENCODER_RCHA);
  RCHB = gpioRead(PIN_ENCODER_RCHB);
  RSWITCH = gpioRead(PIN_ENCODER_RSWITCH);

  if(!falling_edge)
  {
    if(RCHA)
      if(!RCHB)
      {
        direction = RIGHT;
        falling_edge = true;
      }
    else
  }
  else
  {

  }
}

/******************************************************************************/
