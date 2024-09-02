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
    IDLE,
    READ,
    SORT,
    PARSE,
    INVALID
} MagCardState_t;


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
bool MagCardCheckData (void);

/**
 * @brief
 * @param
 * @return
 */
MagCard_t MagCardGetData (void);


/*******************************************************************************
 ******************************************************************************/

#endif // _MAGCARD_H_
