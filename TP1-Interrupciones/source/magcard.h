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

#define CHAR_LENGTH		        4 // bits
#define PARITY_LENGTH	        1

#define MAX_PAN_LENGTH	        19 // chars
#define EXPIRATION_LENGTH       4
#define SERVICE_CODE_LENGTH     3
#define PVKI_LENGTH		        1
#define PVV_LENGTH		        4
#define CVV_LENGTH		        3


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

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
	bool valid; // Serves as a flag
} MagCard_t;


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialize the MagCard driver
 * @return 0 if no error, 1 otherwise
 */
bool MagCardInit (void);

/**
 * @brief Get the status of the MagCard driver
 * @return 0 if no error, 1 otherwise
 */
bool MagCardGetStatus (void);

/**
 * @brief Complete Access to Primary Data
 * @return Pointer to the corresponding MagCard structure
 */
MagCard_t *						MagCardGetData				(void);
MagCardData_t *					MagCardGetPANData			(void);
MagCardAdditionalData_t *		MagCardGetAdditionalData	(void);
MagCardDiscretionaryData_t *	MagCardGetDiscretionaryData	(void);

/**
 * @brief Direct Access to Primary Data Fields
 * @return Value of the corresponding field in the MagCard structure
 */
char *	MagCardGetPAN			(void);
uint8_t	MagCardGetPANLength		(void);
char *	MagCardGetExpiration	(void);
char *	MagCardGetServiceCode	(void);
char *	MagCardGetPVKI			(void);
char *	MagCardGetPVV			(void);
char *	MagCardGetCVV			(void);
char	MagCardGetLRC			(void);
//bool	MagCardIsValid			(void);


/*******************************************************************************
 ******************************************************************************/

#endif // _MAGCARD_H_
