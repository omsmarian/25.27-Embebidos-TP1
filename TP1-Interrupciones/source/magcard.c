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
//#include "SysTick.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define DEVELOPMENT_MODE	1
#define MAX_TRACK_SIZE		200
#define MAX_CHARS			40
#define START_SENTINEL		0xB
#define FIELD_SEPARATOR		0xD
#define END_SENTINEL		0xF
//#define INVALID_DATA		{ 0 }
#define CHAR_LENGTH			4 // bits
#define MAX_PAN_LENGTH		19 // chars
//#define MAX_FS_POS_		1 + MAX_PAN_LENGTH
#define EXPIRATION_LENGTH	4
#define SERVICE_CODE_LENGTH	3
#define PVKI_LENGTH			1
#define PVV_LENGTH			4
#define CVV_LENGTH			3
#define LRC_LENGTH			1


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void FSM					(MagCardEvent_t event);
static bool Init				(void);
static void ReadEnable			(void);
static void ReadClock			(void);
static void ReadBit				(void);
static bool CleanData			(void);
//static bool CheckRowParity	(char character, bool parity);
//static bool CheckColumnParity	(void);
static bool CheckParity			(void);
static bool ParseData			(void);
static char Bits2Char			(bool bits[]);

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static char				data[40]			= { 0 }; // There has to be a better option...
//static bool			parity[40]			= { 0 };
static bool				track_buffer[200]	= { 0 };
static bool *			track2				= NULL;
static uint8_t			index				= 0;
//static bool			data[40][4]			= { { 0 } }; // Would use 1 byte to store 1 bit!!! But is convenient...

//MagCardFlags_t		flags				= { 0 };
static bool				flag_invalid		= false;
static bool				flag_data_ready		= false;

static uint8_t			ss_pos_bits			= 0;
static uint8_t			fs_pos_chars		= 0;
static uint8_t			es_pos_chars		= 0;
static uint8_t			lrc_pos_chars		= 0;
static uint8_t			track2_length		= 0;

static MagCard_t		magCard;
static MagCardState_t	state				= OFF;
//static MagCardEvent_t	event				= NONE;

// Reset function?
// Initialization function or initialize here?
// Store data until retrieved?


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

bool MagCardInit (void)
{
	FSM(INIT);

	return state != OFF; // Correctly initialized
}

bool MagCardGetStatus (void)
{ return magCard.valid && state == IDLE; } // return if data was read or only if it was valid?
										   // A flag would be simpler

//bool MagCardGetStatus (void)
//{
//	FSM(GET_STATUS);
//
//	return state == VALID_DATA_AVAILABLE;
//}

// Complete Data Access

MagCard_t					MagCardGetData				(void) { return magCard; }
MagCardData_t				MagCardGetPANData			(void) { return magCard.data; }
MagCardAdditionalData_t		MagCardGetAdditionalData	(void) { return magCard.additional_data; }
MagCardDiscretionaryData_t	MagCardGetDiscretionaryData	(void) { return magCard.discretionary_data; }

// Direct Field Access

char *						MagCardGetPAN				(void) { return magCard.data.PAN; }
uint8_t						MagCardGetPANLength			(void) { return magCard.data.PAN_length; }
char *						MagCardGetExpiration		(void) { return magCard.additional_data.expiration; }
char *						MagCardGetServiceCode		(void) { return magCard.additional_data.service_code; }
char						MagCardGetPVKI				(void) { return magCard.discretionary_data.PVKI; }
char *						MagCardGetPVV				(void) { return magCard.discretionary_data.PVV; }
char *						MagCardGetCVV				(void) { return magCard.discretionary_data.CVV; }
char						MagCardGetLRC				(void) { return magCard.LRC; }
//bool						MagCardIsValid				(void) { return magCard.valid; }


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void FSM (MagCardEvent_t event)
{
	switch (state)
	{
		case OFF:
			if (event == INIT && Init())
			{
				state = IDLE;
				// Initialize variables here?
			}

			break;

		case IDLE:
			if (event == ENABLE_FallingEdge)
			{
				state = READING;
				index = ss_pos_bits = fs_pos_chars = es_pos_chars = lrc_pos_chars = 0;
				flag_data_ready = flag_invalid = 0;
			}
//			if (event == GET_STATUS)
//				state = VALID_DATA_AVAILABLE; // This depends on what happens to the data
//											  // if another swipe occurs before it was read from App()

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
			magCard.valid = CleanData() && CheckParity() && ParseData();
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
}

static bool Init (void)
{
	bool status = true;

	magCard.valid = false; // Initialized here?

	gpioMode(PIN_MAGCARD_ENABLE, INPUT);
	gpioMode(PIN_MAGCARD_CLOCK, INPUT);
	gpioMode(PIN_MAGCARD_DATA, INPUT);

	if (status)
		status = !gpioIRQ(PIN_MAGCARD_ENABLE, GPIO_IRQ_MODE_BOTH_EDGES, ReadEnable); // Periodic?
	if (status)
		status = !gpioIRQ(PIN_MAGCARD_CLOCK, GPIO_IRQ_MODE_FALLING_EDGE, ReadClock);

	return status; // Should return 1 if there was an error...
}

static void ReadEnable (void)
{ FSM(!gpioRead(PIN_MAGCARD_ENABLE) ? ENABLE_FallingEdge : ENABLE_RisingEdge); } // In this case a flag would be better (faster)

static void ReadClock (void)
{ FSM(CLOCK_FallingEdge); }

static void ReadBit (void)
{
	track_buffer[index++] = !gpioRead(PIN_MAGCARD_DATA); // Data is inverted
	index %= MAX_TRACK_SIZE; // Prevent writing after end of array
}

static bool CleanData (void)
{
	bool status = true;
	char buffer = 0;
	uint8_t i = 0, j = 0;

	while (i < MAX_TRACK_SIZE && buffer != START_SENTINEL) // Search for SS
		buffer = ((buffer >> 1) & 0xF) | (track_buffer[i++] << (CHAR_LENGTH - 1));
	j = i;
	while (i < MAX_TRACK_SIZE && buffer != END_SENTINEL) // Search for ES
		buffer = ((buffer >> 1) & 0xF) | (track_buffer[i++] << (CHAR_LENGTH - 1));

	if (i < MAX_TRACK_SIZE - CHAR_LENGTH - 2) // Even if ES was found at the end, its parity bit and the LRC would be missing
	{
		track2 = &track_buffer[j - CHAR_LENGTH];
		track2_length = i + CHAR_LENGTH + 2;

//		for (uint8_t i = 0; i < 40 && !lrc_pos_chars; i++) // Instead of 40 use the number of elements read
//		{
//			data[i] = 0;
//			for (uint8_t j = 0; j < 4; j++)
//				data[i] |= track_buffer[ss_pos_bits + i * 5 + j] << j;
//			parity[i] = track_buffer[ss_pos_bits + i * 5 + CHAR_LENGTH];
//
//			if(data[i] == FIELD_SEPARATOR)
//				fs_pos_chars = i;
//			else if(data[i] == END_SENTINEL)
//			{
//				es_pos_chars = i;
//				lrc_pos_chars = i + 1; // Always after?
//			}
//		}
	}
	else
		status = false;

//	flag_invalid = es_pos_chars < fs_pos_chars || fs_pos_chars < ss_pos_bits;

	return status;
}

static bool ParseData (void)
{
	bool status = true;
	uint8_t i = 1, j = 0, stop = MAX_TRACK_SIZE - CHAR_LENGTH - 2; // Skip SS and stop at ES

	do {
		magCard.data.PAN[i - 1] = Bits2Char(track2 + i * 4);
	} while(magCard.data.PAN[i++ - 1] != FIELD_SEPARATOR && i < MAX_PAN_LENGTH + 1);
	magCard.data.PAN_length = i - 2; // If > 19?

	if (magCard.data.PAN_length < MAX_PAN_LENGTH ||
	   ((magCard.data.PAN_length == MAX_PAN_LENGTH) && (Bits2Char(track2 + i * 4) == FIELD_SEPARATOR)))
	{
		for (j = i; i < stop && i < j + EXPIRATION_LENGTH; i++)
			magCard.additional_data.expiration[i] = Bits2Char(track2 + i * 4);

		for (j = i; i < stop && i < j + SERVICE_CODE_LENGTH; i++)
			magCard.additional_data.service_code[i] = Bits2Char(track2 + i * 4);

		for (j = i; i < stop && i < j + PVKI_LENGTH; i++)
			magCard.discretionary_data.PVKI = Bits2Char(track2 + i * 4);

		for (j = i; i < stop && i < j + PVV_LENGTH; i++)
			magCard.discretionary_data.PVV[i] = Bits2Char(track2 + i * 4);

		for (j = i; i < stop && i < j + CVV_LENGTH; i++)
			magCard.discretionary_data.CVV[i] = Bits2Char(track2 + i * 4);

		magCard.LRC = data[++i];
	}
	else
		status = false;

	return status;
}

static char Bits2Char (bool bits[])
{
	char buffer = 0;
	for (uint8_t i = 0; i < 4; i++)
		buffer |= bits[i] << i;

	return buffer;
}

//static bool CheckRowParity (char character, bool parity) // Transversal redundancy check
//{
//    uint8_t count = 0;
//    for (uint8_t i = 0; i < CHAR_LENGTH; i++)
//        count += (character >> i) & 1;
//    return count % 2 != parity; // Parity bit is 1 if even
//}
//
//static bool CheckColumnParity (void) // Longitudinal redundancy check
//{
//    uint8_t stop = es_pos_chars / 5 - 1; // Better if this is already calculated (to use chars instead of bits)
//    bool status = true;
//    for (uint8_t i = 0; i < 4; i++)
//    {
//        uint8_t count = data[i] & 1;
//        for (uint8_t j = 0; j < stop; j += 4)
//            count += (data[j] >> j) & 1;
//        if (count % 2 != ((magCard.LRC >> i) & 1))
//            status = false;
//    }
//    return status; // Parity bit is 1 if even
//}

static bool CheckParity (void)
{
	return 1;
}
