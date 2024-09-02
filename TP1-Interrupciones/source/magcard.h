/***************************************************************************//**
  @file     magcard.h
  @brief    Magnetic Stripe Card Reader driver
  @author   Group 4
 ******************************************************************************/

#ifndef _MAGCARD_H_
#define _MAGCARD_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdbool.h>
#include <stdint.h>


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/***** MAGCARD defines ********************************************************/

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef struct {
    bool init;
    bool enable;
    bool sort;
    bool parity;
    bool parse;
    // bool invalid;
} MagCardFlags_t;

typedef struct {
    char PAN [19];
    uint8_t PAN_length;
} MagCardData_t;

typedef struct {
  char expiration [4];
  char service_code [3];
} MagCardAdditionalData_t;

typedef struct {
  char PVKI;
  char PVV[4];
  char CVV[3];
} MagCardDiscretionaryData_t;

typedef struct {
  MagCardData_t data;
  MagCardAdditionalData_t additional_data;
  MagCardDiscretionaryData_t discretionary_data;
  char LRC;
  bool valid;
} MagCard_t;

typedef enum {
	OFF,
    IDLE,
    READING,
    PROCESSING,
	VALID_DATA_AVAILABLE,
} MagCardState_t;

typedef enum {
	NONE,
	INIT,
	ENABLE_FallingEdge,
	ENABLE_RisingEdge,
	CLOCK_FallingEdge,
	GET_STATUS,
	DATA_ACCESSED,
} MagCardEvent_t;

typedef bool bit_t;

typedef uint8_t byte_t;


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief
 * @param
 * @return
 */
bool MagCardInit (void);

/**
 * @brief
 * @param
 * @return
 */
bool MagCardGetStatus (void);

/**
 * @brief
 * @param
 * @return
 */
MagCard_t MagCardGetData (void);


/*******************************************************************************
 ******************************************************************************/

#endif // _MAGCARD_H_
