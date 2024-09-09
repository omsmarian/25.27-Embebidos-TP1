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
#include "fsl_debug_console.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define CHAR_LENGTH		        4 // Bits
#define PARITY_LENGTH	        1

#define SENTINEL_LENGTH			1 // Chars
#define MAX_PAN_LENGTH	        19
#define EXPIRATION_LENGTH       4
#define SERVICE_CODE_LENGTH     3
#define PVKI_LENGTH		        1
#define PVV_LENGTH		        4
#define CVV_LENGTH		        3
#define UNUSED_SPACE			2
#define LRC_LENGTH				1


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

// MagCard Structures, could be private if complete access is not needed or wanted

typedef struct {
	char PAN[MAX_PAN_LENGTH];
	uint8_t PAN_length;
} MagCardData_t;

typedef struct {
	char expiration[EXPIRATION_LENGTH];
	char service_code[SERVICE_CODE_LENGTH];
} MagCardAdditionalData_t;

typedef struct {
	char PVKI[PVKI_LENGTH];
	char PVV[PVV_LENGTH];
	char CVV[CVV_LENGTH];
} MagCardDiscretionaryData_t;

typedef struct {
	MagCardData_t data;
	MagCardAdditionalData_t additional_data;
	MagCardDiscretionaryData_t discretionary_data;
	char LRC;
} MagCard_t;


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

// Primary Driver Services /////////////////////////////////////////////////////

/**
 * @brief Initialize the MagCard driver
 * @return 0 if no error
 * @note This function must be called before any other MagCard function
 * @example if (MagCardInit()) { // Initialization successful }
 */
bool MagCardInit (void);

/**
 * @brief Get the status of the MagCard driver
 * @return 1 if data is ready
 * @note This function should be called before accessing the data
 * @example if (MagCardGetStatus()) { // Data is ready }
 */
bool MagCardGetStatus (void);

/**
 * @brief Get the card number (PAN)
 * @return Card number (16 digits)
 * @example uint64_t card_number = MagCardGetCardNumber();
 */
uint64_t MagCardGetCardNumber (void);

/**
 * @brief Clear the data buffer
 * @note This function should be called after accessing the data
 * @example MagCardClearData();
 */
void MagCardClearData (void); // This could be done automatically after reading the data

// Complete Data Access ////////////////////////////////////////////////////////

/**
 * @brief Complete Access to Primary Data, could be private if not needed or wanted
 * @return Pointer to the corresponding MagCard structure
 * @example MagCard_t * card = MagCardGetData();
 */
MagCard_t *						MagCardGetData				(void);
MagCardData_t *					MagCardGetPANData			(void);
MagCardAdditionalData_t *		MagCardGetAdditionalData	(void);
MagCardDiscretionaryData_t *	MagCardGetDiscretionaryData	(void);

// Direct Field Access /////////////////////////////////////////////////////////

/**
 * @brief Direct Access to Primary Data Fields
 * @return Value of the corresponding field in the MagCard structure
 * @example char * PAN = MagCardGetPAN();
 */
char *	MagCardGetPAN			(void);
uint8_t	MagCardGetPANLength		(void);
char *	MagCardGetExpiration	(void);
char *	MagCardGetServiceCode	(void);
char *	MagCardGetPVKI			(void);
char *	MagCardGetPVV			(void);
char *	MagCardGetCVV			(void);
char	MagCardGetLRC			(void);


/*******************************************************************************
 ******************************************************************************/

#endif // _MAGCARD_H_
