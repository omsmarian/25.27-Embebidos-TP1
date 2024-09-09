/***************************************************************************//**
  @file     magcard.c
  @brief    Magnetic Stripe Card Reader driver
  @author   Group 4
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "magcard.h"
#include "board.h"
#include "gpio.h"
//#include "pisr.h"
#include "macros.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define DEVELOPMENT_MODE		1

#define MAX_CHARS				((SENTINEL_LENGTH * 3) + MAX_PAN_LENGTH + \
								 EXPIRATION_LENGTH + SERVICE_CODE_LENGTH + \
								 PVKI_LENGTH + PVV_LENGTH + CVV_LENGTH + UNUSED_SPACE + LRC_LENGTH)
#define DATA_LENGTH				(CHAR_LENGTH + PARITY_LENGTH)
#define MAX_TRACK_SIZE			(DATA_LENGTH * MAX_CHARS)

#define START_SENTINEL			0xB
#define FIELD_SEPARATOR			0xD
#define END_SENTINEL			0xF

#define HARD_CLEAR				0


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
	ACCES_DATA,
	CLEAR_DATA
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
static void				MagCardCpy		(MagCard_t * dest, MagCard_t * src);
static bool				MagCardClr		(void);

static char				__Bits2Char__	(bool bits[]);
static uint8_t			__StoreBits__	(uint8_t track2_pos, char data[], uint8_t length);
static uint8_t			__StoreChar__	(uint8_t track2_pos, char data[], uint8_t length);
static uint64_t			__CharsToNum__	(char chars[], uint8_t length);
static void				__ArrayCpy__	(char dest[], char src[], uint8_t length);


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static bool				track2[MAX_TRACK_SIZE + 1]; // + 1 in case the data is longer than MAX_TRACK_SIZE (the rest is ignored, index is capped)
static uint8_t			track2_length;
static uint8_t			index;

static MagCard_t		magCard;
static MagCard_t		magCardBuffer; // Prevents data corruption from user
static MagCardState_t	state;


/*******************************************************************************
 *******************************************************************************
						GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

// Primary Driver Services /////////////////////////////////////////////////////

//bool							MagCardGetStatus			(void) { return FSM(GET_STATUS) == DATA_READY; }
bool							MagCardInit					(void) { return !FSM(INIT); } // OFF: 0
bool							MagCardGetStatus			(void) { return FSM(GET_STATUS) == DATA_READY; }
uint64_t						MagCardGetCardNumber		(void) { return __CharsToNum__(magCard.data.PAN, magCard.data.PAN_length); }
void							MagCardClearData			(void) { FSM(CLEAR_DATA); }

// Complete Data Access ////////////////////////////////////////////////////////

MagCard_t *						MagCardGetData				(void) { return &magCardBuffer; }
MagCardData_t *					MagCardGetPANData			(void) { return &magCardBuffer.data; }
MagCardAdditionalData_t *		MagCardGetAdditionalData	(void) { return &magCardBuffer.additional_data; }
MagCardDiscretionaryData_t *	MagCardGetDiscretionaryData	(void) { return &magCardBuffer.discretionary_data; }

// Direct Field Access /////////////////////////////////////////////////////////

char *							MagCardGetPAN				(void) { return magCardBuffer.data.PAN; }
uint8_t							MagCardGetPANLength			(void) { return magCardBuffer.data.PAN_length; }
char *							MagCardGetExpiration		(void) { return magCardBuffer.additional_data.expiration; }
char *							MagCardGetServiceCode		(void) { return magCardBuffer.additional_data.service_code; }
char *							MagCardGetPVKI				(void) { return magCardBuffer.discretionary_data.PVKI; }
char *							MagCardGetPVV				(void) { return magCardBuffer.discretionary_data.PVV; }
char *							MagCardGetCVV				(void) { return magCardBuffer.discretionary_data.CVV; }
char							MagCardGetLRC				(void) { return magCardBuffer.LRC; }
// bool							MagCardIsValid				(void) { return magCardBuffer.valid; }


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
			if (event == ENABLE_FallingEdge) { (state = WAITING_SS) && (index = 0); }
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
			if (event == GET_STATUS) // Check if data is ready
			{
				uint8_t i = 0, buffer = 0;
				for (i++; i < MAX_CHARS && __Bits2Char__(track2 + i * DATA_LENGTH) != FIELD_SEPARATOR; i++); // Search for FS
				magCard.data.PAN_length = i - 1; // SS and FS not included
				for (i++; i < MAX_CHARS && __Bits2Char__(track2 + i * DATA_LENGTH) != END_SENTINEL; i++); // Search for ES
				track2_length = (i + 2) * DATA_LENGTH; // LRC included

				if (magCard.data.PAN_length <= MAX_PAN_LENGTH && i < MAX_CHARS && CheckParity())
				{
					ParseData();
					state = DATA_READY;
				}
				else
					state = IDLE;
			}
			else if (event == ENABLE_FallingEdge) { (state = WAITING_SS) && (index = 0); }
			else if (event == CLEAR_DATA) { (state = IDLE) && MagCardClr(); }
			break;

		case DATA_READY:
				 if (event == ENABLE_FallingEdge) { (state = WAITING_SS) && (index = 0); }
			else if (event == CLEAR_DATA) { (state = IDLE) && MagCardClr(); }
			break;

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

	status = !gpioIRQ(PIN_MAGCARD_ENABLE, GPIO_IRQ_MODE_BOTH_EDGES, ReadEnable);
	if (status)
		status = !gpioIRQ(PIN_MAGCARD_CLOCK, GPIO_IRQ_MODE_FALLING_EDGE, ReadClock);

	return status;
}

static void ReadEnable  (void) {
	DEBUG_TP_SET_D;
	FSM(!gpioRead(PIN_MAGCARD_ENABLE) ? ENABLE_FallingEdge : ENABLE_RisingEdge);
    DEBUG_TP_CLR_D; }
static void ReadClock   (void) {
	DEBUG_TP_SET;
	FSM(CLOCK_FallingEdge);
	DEBUG_TP_CLR; }
static void ReadBit     (void) { track2[index++] = !gpioRead(PIN_MAGCARD_DATA); } // Data is inverted

static bool CheckParity (void)
{
	bool parity = 1; // Odd parity

	for (int i = 0; i < track2_length && parity; i += DATA_LENGTH) // Parity included
	{
		parity = track2[i];
		for (int j = 1; j < DATA_LENGTH; j++)
			parity ^= track2[i + j];
	}

	if (parity)
	{
		parity = 0; // Even parity
		for (int j = 0; j < CHAR_LENGTH && !parity; j++) // Parity not included
		{
			parity = track2[j];
			for (int i = DATA_LENGTH; i < track2_length; i += DATA_LENGTH)
				parity ^= track2[i + j];
		}
		parity = !parity;
	}

	return parity;
}

static void ParseData (void)
{
	uint8_t i = DATA_LENGTH; // Skip SS

	i = __StoreChar__(i, magCard.data.PAN,						magCard.data.PAN_length) + DATA_LENGTH; // Skip FS
	i = __StoreChar__(i, magCard.additional_data.expiration,	EXPIRATION_LENGTH);
	i = __StoreChar__(i, magCard.additional_data.service_code,	SERVICE_CODE_LENGTH);
	i = __StoreChar__(i, magCard.discretionary_data.PVKI,		PVKI_LENGTH);
	i = __StoreChar__(i, magCard.discretionary_data.PVV,		PVV_LENGTH);
	i = __StoreChar__(i, magCard.discretionary_data.CVV,		CVV_LENGTH) + DATA_LENGTH; // Skip ES
	i = __StoreChar__(i, &magCard.LRC,							LRC_LENGTH);

	MagCardCpy(&magCardBuffer, &magCard);
}

static void MagCardCpy (MagCard_t * dest, MagCard_t * src)
{
	dest->data.PAN_length = src->data.PAN_length;

	__ArrayCpy__(dest->data.PAN,						src->data.PAN,						src->data.PAN_length);
	__ArrayCpy__(dest->additional_data.expiration,		src->additional_data.expiration,	EXPIRATION_LENGTH);
	__ArrayCpy__(dest->additional_data.service_code,	src->additional_data.service_code,	SERVICE_CODE_LENGTH);
	__ArrayCpy__(dest->discretionary_data.PVKI,			src->discretionary_data.PVKI,		PVKI_LENGTH);
	__ArrayCpy__(dest->discretionary_data.PVV,			src->discretionary_data.PVV,		PVV_LENGTH);
	__ArrayCpy__(dest->discretionary_data.CVV,			src->discretionary_data.CVV,		CVV_LENGTH);

	dest->LRC = src->LRC;
}

static bool MagCardClr (void)
{
#ifdef HARD_CLEAR
	magCard.data.PAN_length = 0;

	char buffer[MAX_CHARS] = {0};
	MagCardCpy(&magCard, buffer);

	magCard.LRC = 0;

	MagCardCpy(&magCardBuffer, &magCard);
#endif
	return 0; // Clear successful
}

// Helper Functions ////////////////////////////////////////////////////////////

static char __Bits2Char__ (bool bits[])
{
	char buffer = 0;
	for (uint8_t i = 0; i < CHAR_LENGTH; i++)
		buffer |= bits[i] << i;

	return buffer;
}

static bool * __Char2Bits__ (char c)
{
	bool bits[DATA_LENGTH];
	for (uint8_t i = 0; i < DATA_LENGTH; i++)
		bits[i] = c & (1 << i);

	return bits;
}

static uint64_t __CharsToNum__ (char chars[], uint8_t length)
{
	uint64_t buffer = 0;
	for (uint8_t i = 0; i < length; i++)
		buffer = buffer * 10 + ASCII2CHAR(chars[i]);

	return buffer;
}

static void __ArrayCpy__ (char dest[], char src[], uint8_t length)
{
	for (uint8_t i = 0; i < length; i++)
		dest[i] = src[i];
}

static uint8_t __StoreBits__ (uint8_t track2_pos, char data[], uint8_t length)
{
	for (uint8_t j = 0; track2_pos < track2_length && j < length; track2_pos++, j++)
		__ArrayCpy__(track2 + track2_pos, __Char2Bits__(ASCII2CHAR(data[j])), DATA_LENGTH);

	return track2_pos;
}

static uint8_t __StoreChar__ (uint8_t track2_pos, char field[], uint8_t length)
{
	for (uint8_t j = 0; track2_pos < track2_length && j < length; track2_pos += DATA_LENGTH, j++)
		field[j] = CHAR2ASCII(__Bits2Char__(track2 + track2_pos));

	return track2_pos;
}


/******************************************************************************/
