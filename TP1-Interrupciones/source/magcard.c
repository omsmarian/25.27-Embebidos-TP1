/***************************************************************************//**
  @file     magcard.c
  @brief    Magnetic Stripe Card Reader driver
  @author   Group 4
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdlib.h>
#include "magcard.h"
#include "board.h"
#include "gpio.h"
//#include "pisr.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define DEVELOPMENT_MODE        1

#define MAX_CHARS		        MAX_PAN_LENGTH + EXPIRATION_LENGTH + SERVICE_CODE_LENGTH + PVKI_LENGTH + PVV_LENGTH + CVV_LENGTH + 4
#define DATA_LENGTH		        CHAR_LENGTH + PARITY_LENGTH
#define MAX_TRACK_SIZE	        DATA_LENGTH * MAX_CHARS

#define START_SENTINEL	        0xB
#define FIELD_SEPARATOR	        0xD
#define END_SENTINEL	        0xF

#define BITINSERT_RIGHT(x, b)   (x = x << 1 | (b << (CHAR_LENGTH - 1)));
#define BITINSERT_LEFT(x, b)    (x = (x >> 1) | (b << (CHAR_LENGTH - 1)));


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum {
	OFF,
	IDLE,
	WAITING_SS,
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


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static MagCardState_t   FSM		    	(MagCardEvent_t event);
static bool             Init	    	(void);
static void             ReadEnable  	(void);
static void             ReadClock   	(void);
static void             ReadBit	    	(void);
static bool             CheckParity 	(void);
static void             ParseData   	(void);
static char             __Bits2Char__   (bool bits[]);
static uint8_t          __StoreChar__   (uint8_t track2_pos, char data[], uint8_t length);


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static bool				track2[200]		= { 0 };
static uint8_t			track2_length	= 0;
static uint8_t			index			= 0;
//static bool			data[40][4]		= { { 0 } }; // Another way to store data, easier to access and check

static MagCard_t		magCard;
static MagCardState_t	state			= OFF;
//static MagCardEvent_t	event			= NONE;


/*******************************************************************************
 *******************************************************************************
						GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

//bool MagCardGetStatus (void) // return if data was read or only if it was valid?
//{
//	FSM(GET_STATUS);
//
//	return state == VALID_DATA_AVAILABLE;
//}

bool                            MagCardInit                 (void) { return !FSM(INIT); } // OFF: 0
bool                            MagCardGetStatus            (void) { return magCard.valid && state == IDLE; }

// Complete Data Access

MagCard_t *						MagCardGetData				(void) { return &magCard; }
MagCardData_t *					MagCardGetPANData			(void) { return &magCard.data; }
MagCardAdditionalData_t *		MagCardGetAdditionalData	(void) { return &magCard.additional_data; }
MagCardDiscretionaryData_t *    MagCardGetDiscretionaryData	(void) { return &magCard.discretionary_data; }

// Direct Field Access

char *							MagCardGetPAN				(void) { return magCard.data.PAN; }
uint8_t							MagCardGetPANLength			(void) { return magCard.data.PAN_length; }
char *							MagCardGetExpiration		(void) { return magCard.additional_data.expiration; }
char *							MagCardGetServiceCode		(void) { return magCard.additional_data.service_code; }
char *							MagCardGetPVKI				(void) { return magCard.discretionary_data.PVKI; }
char *							MagCardGetPVV				(void) { return magCard.discretionary_data.PVV; }
char *							MagCardGetCVV				(void) { return magCard.discretionary_data.CVV; }
char							MagCardGetLRC				(void) { return magCard.LRC; }
//bool							MagCardIsValid				(void) { return magCard.valid; }


/*******************************************************************************
 *******************************************************************************
						LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static MagCardState_t FSM (MagCardEvent_t event)
{
	switch (state)
	{
		case OFF:
			if (event == INIT && Init())
				state = IDLE;

			break;

		case IDLE:
			if (event == ENABLE_FallingEdge)
			{
				state = READING;
				magCard.valid = false;
				index = 0;
			}
//			if (event == GET_STATUS)
//				state = VALID_DATA_AVAILABLE; // This depends on what happens to the data
//											  // if another swipe occurs before it was read from App()

			break;

		case WAITING_SS: // Same as READING but without storing data
			if (event == CLOCK_FallingEdge)
			{
				static char buffer = 0;
				ReadBit();
				BITINSERT_LEFT(buffer, track2[0]);
				buffer >>= track2[0];
				if (buffer == START_SENTINEL) // Search for SS
					state = READING;
				index = 0;

//				__Bits2Char__(track2);
//
//				bool flag = false;
//				for (uint8_t i = 0; i < CHAR_LENGTH && flag; i++)
//					flag = (START_SENTINEL & (1 << i)) >> i == track2[i];
//				if (flag)
//					state = READING;
//
//				char buffer = 0;
//				while (i < CHAR_LENGTH && buffer != START_SENTINEL) // Search for SS
//					buffer = ((buffer >> 1) & 0xF) | (track2[i++] << (CHAR_LENGTH - 1));
//				if (buffer == START_SENTINEL)
//					state = READING;
			}
			else if (event == ENABLE_RisingEdge)
				state = IDLE;

			break;

		case READING:
			if (event == CLOCK_FallingEdge)
				ReadBit();
			else if (event == ENABLE_RisingEdge)
				state = PROCESSING;

			break;

		case PROCESSING:
////			if (CleanData())
////			{
//			if (CleanData() && (magCard.valid = CheckParity()) )
//				ParseData();
////			if (CheckParity())
////				state = VALID_DATA_AVAILABLE;
////			else
////				state = IDLE; // If data is ready, should it wait for it to be accessed or continue reading?
////				magCard.valid = CheckParity();
////			}
//			else
//				magCard.valid = false;

			uint8_t i = 0, buffer = 0;
			do { buffer = __Bits2Char__(track2 + i * 5); }
			while (i++ < MAX_TRACK_SIZE && buffer != FIELD_SEPARATOR); // Search for FS
			magCard.data.PAN_length = i * 5 - 1; // SS not included
			if (buffer == FIELD_SEPARATOR && magCard.data.PAN_length <= MAX_PAN_LENGTH)
			{
				while (i++ < MAX_TRACK_SIZE && __Bits2Char__(track2 + i * 5) != END_SENTINEL); // Search for ES
				if(i < MAX_TRACK_SIZE - CHAR_LENGTH - 2) // Even if ES was found at the end, its parity bit and the LRC would be missing
				{
					track2_length = i + CHAR_LENGTH + 2;
					magCard.valid = CheckParity();
					ParseData();
				}
			}
			else
				magCard.valid = false;

			state = IDLE;

			break;

//		case VALID_DATA_AVAILABLE: // Maybe a flag would be better
//			if (event == DATA_ACCESSED)
//				state = IDLE;
//
//			break;

//			case VALID_DATA_AVAILABLE:
//				if (event == DATA_ACCESSED)
//					state = IDLE;
//				else if (event = ENABLE_FallingEdge)
//
//				break;

		default:
			state = OFF;

			break;
	}

	return state;
}

static bool Init (void)
{
	bool status = true;

	magCard.valid = false;

	gpioMode(PIN_MAGCARD_ENABLE, INPUT);
	gpioMode(PIN_MAGCARD_CLOCK, INPUT);
	gpioMode(PIN_MAGCARD_DATA, INPUT);

	status = !gpioIRQ(PIN_MAGCARD_ENABLE, GPIO_IRQ_MODE_BOTH_EDGES, ReadEnable); // Periodic?
	if (status)
		status = !gpioIRQ(PIN_MAGCARD_CLOCK, GPIO_IRQ_MODE_FALLING_EDGE, ReadClock);

	return status; // Should return 1 if there was an error...
}

static void ReadEnable  (void) { FSM(!gpioRead(PIN_MAGCARD_ENABLE) ? ENABLE_FallingEdge : ENABLE_RisingEdge); }
static void ReadClock   (void) { FSM(CLOCK_FallingEdge); }
static void ReadBit     (void) { track2[index++] = !gpioRead(PIN_MAGCARD_DATA); } // Data is inverted

static bool CheckParity (void)
{
	bool status = true;

	uint32_t cols[DATA_LENGTH] = { 0 }; // Parity included
	for (uint8_t i = 0; i < DATA_LENGTH; i++)
		for (uint8_t j = 0; j * 5 < track2_length; j++)
			cols[i] |= (track2[i + j * 5]) << j;

	uint32_t buffer = 0;
	for (uint8_t i = 0; i < DATA_LENGTH; i++)
		buffer ^= cols[i];

	if (!buffer)
	{
		uint8_t rows[MAX_TRACK_SIZE] = { 0 }; // Parity not included
		for (uint8_t j = 0; j * 5 < track2_length; j++)
			for (uint8_t i = 0; i < CHAR_LENGTH; i++)
				rows[j] |= (track2[i + j * 5]) << i;

		uint8_t buffer = 0;
		for (uint8_t i = 0; i < track2_length; i++)
			buffer ^= rows[i];
		
		if (buffer)
			status = false;
	}
	else
		status = false;

	return status;
}

static void ParseData (void)
{
	uint8_t i = 1; // Skip SS

	i = __StoreChar__(i, magCard.data.PAN, magCard.data.PAN_length) + DATA_LENGTH; // Skip FS
	i = __StoreChar__(i, magCard.additional_data.expiration, EXPIRATION_LENGTH);
	i = __StoreChar__(i, magCard.additional_data.service_code, SERVICE_CODE_LENGTH);
	i = __StoreChar__(i, magCard.discretionary_data.PVKI, PVKI_LENGTH);
	i = __StoreChar__(i, magCard.discretionary_data.PVV, PVV_LENGTH);
	i = __StoreChar__(i, magCard.discretionary_data.CVV, CVV_LENGTH) + DATA_LENGTH; // Skip ES
	i = __StoreChar__(i, &magCard.LRC, 1);
}

// Helper Functions ////////////////////////////////////////////////////////////

static char __Bits2Char__ (bool bits[])
{
	char buffer = 0;
	for (uint8_t i = 0; i < CHAR_LENGTH; i++)
		buffer |= bits[i] << i;

	return buffer;
}

static uint8_t __StoreChar__ (uint8_t track2_pos, char field[], uint8_t length) // Could be a MACRO?
{
	for (uint8_t j = 0; track2_pos < track2_length && track2_pos < j + length; track2_pos+=DATA_LENGTH, j++)
		field[j] = __Bits2Char__(track2 + track2_pos);

	return track2_pos;
}

// Notes ///////////////////////////////////////////////////////////////////////

// Reset function?
// Initialization function or initialize here?
// Store data until retrieved?
// Check if data is valid before storing it?
// Should Parse() check if data is valid even if parity and LRC is correct?
// Search for FS needed?
