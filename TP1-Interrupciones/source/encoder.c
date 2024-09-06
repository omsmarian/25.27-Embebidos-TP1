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
#define MAX_BUFFER_SIZE 256


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
/**
 * @brief Checks for switch gesture
 */
static void switchCallback(void);

/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static encoder_data_t value;
static encoder_data_t arr[MAX_BUFFER_SIZE];
static state_flags_t encoder_state;
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

  if(pisrRegister(directionCallback, 1) && pisrRegister(switchCallback, 100))
    return 1;
  else
    return 0;
}


encoder_data_t encoderRead(void)
{
  uint16_t aux = value.action_counter;
  for(int i = 0; i < aux; i++)
  {
    if()
    
  }
  // encoder_data_t aux = value;
  // value.action_counter = 0;
  // if(aux.action_counter > 0)
  // {
  //   aux.direction = RIGHT;
  // }
  // else if(aux.action_counter < 0)
  // {
  //   aux.direction = LEFT;
  //   aux.action_counter = -aux.action_counter;
  // }
  // return aux;
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
  encoder_state.RCHA = gpioRead(PIN_ENCODER_RCHA);
  encoder_state.RCHB = gpioRead(PIN_ENCODER_RCHB);

  if (!falling_edge)
  {
    if (encoder_state.RCHA)
    {
      if(!encoder_state.RCHB)
      {
        falling_edge = true;
        value.action_counter++;
        arr[value.action_counter].direction = LEFT;
      }
    }
    else
    {
      if(encoder_state.RCHB)
      {
        falling_edge = true;
        value.action_counter++;
        arr[value.action_counter].direction = RIGHT;
      }
    }
  }
  else
  {
    if (encoder_state.RCHA && encoder_state.RCHB)
    {
      falling_edge = false;
    }
  }
}


static void switchCallback(void)
{
  encoder_state.RSWITCH = gpioRead(PIN_ENCODER_RSWITCH);
  if(!switch_falling_edge)
  {
    if(!encoder_state.RSWITCH)
    {
      switch_falling_edge = true;
      press_duration = 0;
    }
  }
  else
  {
    press_duration++;
    if(encoder_state.RSWITCH)
    {
      switch_falling_edge = false;
      if (press_duration >= LONG_CLICK_THRESHOLD)
      {
          value.action_counter++;
          arr[value.action_counter].direction = LONG_CLICK;
          long_click_detected = true;
      }
      else if (!long_click_detected)
      {
        value.action_counter++;
        arr[value.action_counter].direction = CLICK;
      }
      long_click_detected = false;
    }
  }
}
/******************************************************************************/
