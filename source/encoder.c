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
#define SPIN_FREQ 1000U
#define CLICK_FREQ 20U
#define LONG_CLICK_THRESHOLD 10
#define DOUBLE_CLICK_THRESHOLD 15 // Define el umbral para el doble clic
#define MIN_TIME_BETWEEN_CLICKS 5 // Define el tiempo mínimo entre clics para un doble clic

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

static action_t direction;
static state_flags_t encoder_state;
static bool falling_edge;
static bool switch_falling_edge;
static bool long_click_detected;
static uint8_t press_duration;
static uint8_t click_count = 0; // Contador de clics
static uint8_t click_timer = 0; // Temporizador de clics
static uint8_t time_since_last_click = 0; // Tiempo desde el último clic
static bool click_detected = false; // Variable para detectar si se ha registrado un clic


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

  pisrRegister(directionCallback, PISR_FREQUENCY_HZ/SPIN_FREQ);   // 1 ms 
  pisrRegister(switchCallback, PISR_FREQUENCY_HZ/CLICK_FREQ);    // 50 ms
  direction = NONE;

  return 1;
}


action_t encoderRead(void)
{
  return direction;
}

void ResetEncoder(void)
{
  direction = NONE;
}
/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

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
        direction = LEFT;
        falling_edge = true;
      }
    }
    else
    {
      if(encoder_state.RCHB)
      {
        direction = RIGHT;
        falling_edge = true;
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
      click_count++;
      if (click_count == 1)
      {
          click_timer = DOUBLE_CLICK_THRESHOLD;
          click_detected = true; // Se detectó un clic
      }
    }
  }
  else
  {
    press_duration++;
    if(encoder_state.RSWITCH)
    {
      switch_falling_edge = false;
      if (press_duration >= LONG_CLICK_THRESHOLD && !click_detected) // Verificar si no se ha detectado un clic
      {
          direction = LONG_CLICK;
          long_click_detected = true;
          click_count = 0;
      }
      else if (!long_click_detected)
      {
          if (click_count >= 2)
          {
              direction = DOUBLE_CLICK;
              click_count = 0;
          }
      }
      long_click_detected = false;
      click_detected = false;
    }
  }
  if (click_timer > 0)
  {
      click_timer--;
      if (click_timer == 0)
      {
          if (click_count == 1)
          {
        	  if(!encoder_state.RSWITCH)
        		  direction = LONG_CLICK;
        	  else
        		  direction = CLICK;
          }
          click_count = 0;
      }
  }
}

/******************************************************************************/
