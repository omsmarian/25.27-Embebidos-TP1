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
#define LONG_CLICK_THRESHOLD 15


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
static void directionCallback(void);
static void switchCallback(void);

/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static action_t direction;
static bool RCHA;
static bool RCHB;
static bool RSWITCH;
static bool falling_edge;
static bool switch_falling_edge;
static bool long_click_detected;
static uint8_t press_duration;

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

  pisrRegister(directionCallback, 1);
  pisrRegister(switchCallback, 100);

  return 0;
}


action_t encoderRead(void)
{
  return direction;
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
static void directionCallback(void)
{
  RCHA = gpioRead(PIN_ENCODER_RCHA);
  RCHB = gpioRead(PIN_ENCODER_RCHB);

  if(!falling_edge)
  {
    if(RCHA)
    {
      if(!RCHB)   // Izquierda
      {
        direction = LEFT;
        falling_edge = true;
      }
      else
        direction = NONE;
    }
    else
      if(RCHB)  // Derecha
      {
        direction = RIGHT;
        falling_edge = true;
      }
  }
  else
  {
    if((RCHA == 1) && (RCHB == 1))
      falling_edge = false;
  }
}


static void switchCallback(void)
{
  RSWITCH = gpioRead(PIN_ENCODER_RSWITCH);
  if(!switch_falling_edge)
  {
    if(!RSWITCH)
    {
      switch_falling_edge = true;
      press_duration = 0;
    }
  }
  else
  {
    press_duration++;
    if(RSWITCH)
    {
      switch_falling_edge = false;
      if (press_duration >= LONG_CLICK_THRESHOLD)
      {
          direction = LONG_CLICK;
          long_click_detected = true;
      }
      else if (!long_click_detected)
      {
          direction = CLICK;
      }
      long_click_detected = false;
    }
  }
}
/******************************************************************************/
