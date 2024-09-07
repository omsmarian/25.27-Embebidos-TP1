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

#define DEVELOPMENT_MODE		1

#define MAX_CHARS				(MAX_PAN_LENGTH + \
								 EXPIRATION_LENGTH + SERVICE_CODE_LENGTH + \
								 PVKI_LENGTH + PVV_LENGTH + CVV_LENGTH + UNUSED_SPACE + 4)
#define DATA_LENGTH				(CHAR_LENGTH + PARITY_LENGTH)
#define MAX_TRACK_SIZE			(DATA_LENGTH * MAX_CHARS)

#define START_SENTINEL			0xB
#define FIELD_SEPARATOR			0xD
#define END_SENTINEL			0xF

#define BITROLL_LEFT(x, b)		(x = ((x << 1) & 0xF) | (b & 1))
#define BITROLL_RIGHT(x, b)		(x = (x >> 1) | ((b & 1) << (CHAR_LENGTH - 1)))
#define CAP(x, min, max)		(x = (x < (min) ? (min) : (x > (max) ? (max) : x)))
#define CHAR2ASCII(c)			(c + 48)
#define ASCII2CHAR(c)			(c - 48)


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum {
	OFF,
	IDLE,
	WAITING_SS,
	READING,
	PROCESSING,
	DATA_READY,
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

static MagCardState_t	FSM				(MagCardEvent_t event);
static bool				Init			(void);
static void				ReadEnable		(void);
static void				ReadClock		(void);
static void				ReadBit			(void);
static bool				CheckParity		(void);
static void				ParseData		(void);
static char				__Bits2Char__	(bool bits[]);
//static uint8_t		__StoreBits__	(uint8_t track2_pos, char data[], uint8_t length);
static uint8_t			__StoreChar__	(uint8_t track2_pos, char data[], uint8_t length);
static uint64_t			__CharsToNum__	(char chars[], uint8_t length);


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static bool				track2[MAX_TRACK_SIZE + 1]	= { 0 };
static uint8_t			track2_length				= 0;
static uint8_t			index						= 0;
//static bool			data[40][4]					= { { 0 } }; // Another way to store data, easier to access and check

static MagCard_t		magCard;
static MagCardState_t	state						= OFF;
//static MagCardEvent_t	event						= NONE;

static bool				ready						= false;

/*******************************************************************************
 *******************************************************************************
						GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

bool							MagCardInit					(void) { return !FSM(INIT); } // OFF: 0
//bool							MagCardGetStatus			(void) { return FSM(GET_STATUS) == DATA_READY; }
bool							MagCardGetStatus			(void) { return FSM(GET_STATUS) == IDLE && ready; }
uint64_t						MagCardGetCardNumber		(void) { return __CharsToNum__(magCard.data.PAN, magCard.data.PAN_length); }

// Complete Data Access

MagCard_t *						MagCardGetData				(void) { return &magCard; }
MagCardData_t *					MagCardGetPANData			(void) { return &magCard.data; }
MagCardAdditionalData_t *		MagCardGetAdditionalData	(void) { return &magCard.additional_data; }
MagCardDiscretionaryData_t *	MagCardGetDiscretionaryData	(void) { return &magCard.discretionary_data; }

// Direct Field Access

char *							MagCardGetPAN				(void) { return magCard.data.PAN; }
uint8_t							MagCardGetPANLength			(void) { return magCard.data.PAN_length; }
char *							MagCardGetExpiration		(void) { return magCard.additional_data.expiration; }
char *							MagCardGetServiceCode		(void) { return magCard.additional_data.service_code; }
char *							MagCardGetPVKI				(void) { return magCard.discretionary_data.PVKI; }
char *							MagCardGetPVV				(void) { return magCard.discretionary_data.PVV; }
char *							MagCardGetCVV				(void) { return magCard.discretionary_data.CVV; }
char							MagCardGetLRC				(void) { return magCard.LRC; }


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
			if (event == INIT && Init()) { state = IDLE; }
			break;

		case IDLE:
			if (event == ENABLE_FallingEdge)
			{
				state = WAITING_SS;
				index = 0;
				ready = false;
			}
			break;

		case WAITING_SS: // Same as READING but without storing data (only SS if found)
			if (event == CLOCK_FallingEdge)
			{
				static char buffer = 0;
				ReadBit();
				BITROLL_RIGHT(buffer, track2[0]);
				if (buffer == START_SENTINEL) // Search for SS
				{
					state = READING;
					for (uint8_t i = 0; i < CHAR_LENGTH; i++)
						track2[i] = buffer & (1 << i);
					buffer = 0;
					index = CHAR_LENGTH;
				}
				else
					index = 0;
			}
			else if (event == ENABLE_RisingEdge) { state = IDLE; }
			break;

		case READING:
			if (event == CLOCK_FallingEdge)
			{
				ReadBit();
				CAP(index, 0, MAX_TRACK_SIZE);
			}
			else if (event == ENABLE_RisingEdge) { state = PROCESSING; }
			break;

		case PROCESSING:
			uint8_t i = 0, buffer = 0;
			do { buffer = __Bits2Char__(track2 + i * DATA_LENGTH); }
			while (i++ < MAX_TRACK_SIZE && buffer != FIELD_SEPARATOR); // Search for FS
			magCard.data.PAN_length = i - 2; // SS and FS not included
			if (buffer == FIELD_SEPARATOR && magCard.data.PAN_length <= MAX_PAN_LENGTH)
			{
				while (i++ < MAX_CHARS && __Bits2Char__(track2 + i * DATA_LENGTH) != END_SENTINEL)
				{
					uint8_t a = __Bits2Char__(track2 + i * DATA_LENGTH); // Search for ES
					if (a == END_SENTINEL || i == 37)
						buffer = 3;
				}
				if(i < MAX_CHARS) // LRC included
				{
					track2_length = (i + 1) * DATA_LENGTH;
					
					if (CheckParity())
					{
						ParseData();
//						state = DATA_READY;
						ready = true;
						state = IDLE;
					}
					else
						state = IDLE;
				}
			}
			else
				state = IDLE;
			break;

//		case DATA_READY:
//			if (event == ENABLE_FallingEdge) { state = IDLE; }
//			break;

		default:
			state = OFF;
			break;
	}

	return state;
}

static bool Init (void)
{
	bool status = true;

	gpioMode(PIN_MAGCARD_ENABLE, INPUT);
	gpioMode(PIN_MAGCARD_CLOCK, INPUT);
	gpioMode(PIN_MAGCARD_DATA, INPUT);

	status = !gpioIRQ(PIN_MAGCARD_ENABLE, GPIO_IRQ_MODE_BOTH_EDGES, ReadEnable); // Periodic?
	if (status)
		status = !gpioIRQ(PIN_MAGCARD_CLOCK, GPIO_IRQ_MODE_FALLING_EDGE, ReadClock);

	return status;
}

static void ReadEnable  (void) { FSM(!gpioRead(PIN_MAGCARD_ENABLE) ? ENABLE_FallingEdge : ENABLE_RisingEdge); }
static void ReadClock   (void) { FSM(CLOCK_FallingEdge); }
static void ReadBit     (void) { track2[index++] = !gpioRead(PIN_MAGCARD_DATA); } // Data is inverted

// static bool FindSentinel (uint8_t sentinel)
// {
// 	bool status = false;

// 	uint8_t i = 0, buffer = 0;
// 	do { buffer = __Bits2Char__(track2 + i * DATA_LENGTH); }
// 	while (i++ < MAX_TRACK_SIZE && buffer != sentinel);

// 	if (buffer == sentinel)
// 		status = true;

// 	return status;
// }

// static bool FindSS (void) { return FindSentinel(START_SENTINEL); }

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

	if (!(~buffer))
	{
		uint8_t rows[MAX_CHARS] = { 0 }; // Parity not included
		for (uint8_t j = 0; j * 5 < track2_length; j++)
			for (uint8_t i = 0; i < CHAR_LENGTH; i++)
				rows[j] |= (track2[i + j * 5]) << i;

		uint8_t buffer = 0;
		for (uint8_t i = 0; i < MAX_CHARS; i++)
			buffer ^= rows[i];
		
		if (!(~buffer))
			status = false;
	}
	else
		status = false;

	return status;
}

static void ParseData (void)
{
	uint8_t i = DATA_LENGTH; // Skip SS

	i = __StoreChar__(i, magCard.data.PAN, magCard.data.PAN_length) + DATA_LENGTH; // Skip FS
	i = __StoreChar__(i, magCard.additional_data.expiration, EXPIRATION_LENGTH);
	i = __StoreChar__(i, magCard.additional_data.service_code, SERVICE_CODE_LENGTH);
	i = __StoreChar__(i, magCard.discretionary_data.PVKI, PVKI_LENGTH);
	i = __StoreChar__(i, magCard.discretionary_data.PVV, PVV_LENGTH);
	i = __StoreChar__(i, magCard.discretionary_data.CVV, CVV_LENGTH) + DATA_LENGTH; // Skip ES
	i = __StoreChar__(i, &magCard.LRC, 1);
}

// Helper Functions //////////////////////////////////////////////////////////// // Could be MACROs?

static char __Bits2Char__ (bool bits[])
{
	char buffer = 0;
	for (uint8_t i = 0; i < CHAR_LENGTH; i++)
		buffer |= bits[i] << i;

	return buffer;
}

// static uint8_t __StoreBits__ (uint8_t track2_pos, char field[], uint8_t length)
// {
// 	for (uint8_t j = 0; track2_pos < track2_length && j < length; track2_pos++, j++)
// 		field[j] = __Bits2Char__(track2 + track2_pos);

// 	return track2_pos;
// }

static uint8_t __StoreChar__ (uint8_t track2_pos, char field[], uint8_t length)
{
	for (uint8_t j = 0; track2_pos < track2_length && j < length; track2_pos+=DATA_LENGTH, j++)
		field[j] = CHAR2ASCII(__Bits2Char__(track2 + track2_pos));

	return track2_pos;
}

static uint64_t __CharsToNum__ (char chars[], uint8_t length)
{
	uint64_t buffer = 0;
	for (uint8_t i = 0; i < length; i++)
		buffer = buffer * 10 + ASCII2CHAR(chars[i]);

	return buffer;
}

// Notes ///////////////////////////////////////////////////////////////////////

// Reset function?
// Initialization function or initialize here?
// Store data until retrieved?
// Check if data is valid before storing it?
// Should Parse() check if data is valid even if parity and LRC is correct?
// Search for FS needed?
// Some of the xxxxx < track_length, etc. are not needed (after validation), but its safer this way
// How to distinguish discretionary data?
// Return data if it was read or only if it was valid?
// Should it wait for data to be accessed or continue reading?
// DATA_READY state needed?
