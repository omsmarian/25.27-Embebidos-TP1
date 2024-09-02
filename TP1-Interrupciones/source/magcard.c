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
//#include "SysTick.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define DEVELOPMENT_MODE	1
#define MAX_TRACK_SIZE		200
#define START_SENTINEL		0xB
#define FIELD_SEPARATOR		0xD
#define END_SENTINEL		0xF
#define INVALID_DATA		{ 0 }
#define CHAR_LENGTH			4
#define MAX_PAN_LENGTH		19
#define MAX_fs_pos_chars	1 + MAX_PAN_LENGTH


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void ReadEnable (void);
static void ReadBit (void);
static void CleanData (void);
bool CheckRowParity (char character, bool parity);
bool CheckColumnParity (void);
static void ParseData (void);


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static char data[40]			= { 0 }; // There has to be a better option...
static bool parity[40]			= { 0 };
static bool track_buffer[200]	= { 0 };
static uint8_t index = 0;

// static bool data[40][4] = { { 0 } }; // Would use 1 byte to store 1 bit!!! But is convenient...

// MagCardFlags_t flags = { 0 };
static bool flag_init			= false;
static bool flag_enable			= false;
static bool flag_enable_prev	= false;
static bool flag_sort			= false;
static bool flag_parity			= false;
static bool flag_parse			= false;
static bool flag_invalid		= false;
static bool flag_data_ready 	= false;

static uint8_t ss_pos_bits			= 0;
static uint8_t fs_pos_chars			= 0;
static uint8_t es_pos_chars			= 0;
static uint8_t lrc_pos_chars		= 0;


static MagCard_t magCard;
static MagCardState_t state = IDLE;

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
	bool status = false;

	if (flag_init != true) // Not initialized yet
	{
		status = flag_init = true;
		magCard.valid = false; // Initialized here?

		gpioMode(PIN_MAGCARD_ENABLE, INPUT);
		gpioMode(PIN_MAGCARD_CLOCK, INPUT);
		gpioMode(PIN_MAGCARD_DATA, INPUT);

		if (status)
			status = !gpioIRQ(PIN_MAGCARD_ENABLE, GPIO_IRQ_MODE_BOTH_EDGES, ReadEnable); // Periodic?
		if (status)
			status = !gpioIRQ(PIN_MAGCARD_CLOCK, GPIO_IRQ_MODE_FALLING_EDGE, ReadBit);
	}

	return status; // Should return 1 if there was an error
}

bool MagCardCheckData (void) // Needed? Or return empty data in MagCardGetData (and call it MagCardRead)?
{
	if (!flag_init)
		magCard.valid = false; // magCard may not be initialized
	else if (flag_enable_prev && !flag_data_ready) // Rising Edge (data was read but not processed)
	{
		CleanData();
		if (magCard.valid)
			ParseData();
		flag_data_ready = true;
	}

	return magCard.valid; // return if data was read or only if it was valid?
}

// Complete Data Access

MagCard_t MagCardGetData (void)
{ return magCard; }

MagCardData_t MagCardGetPANData (void)
{ return magCard.data; }

MagCardAdditionalData_t MagCardGetAdditionalData (void)
{ return magCard.additional_data; }

MagCardDiscretionaryData_t MagCardGetDiscretionaryData (void)
{ return magCard.discretionary_data; }

// Direct Field Access

char * MagCardGetPAN (void)
{ return magCard.data.PAN; }

uint8_t MagCardGetPANLength (void)
{ return magCard.data.PAN_length; }

char * MagCardGetExpiration (void)
{ return magCard.additional_data.expiration; }

char * MagCardGetServiceCode (void)
{ return magCard.additional_data.service_code; }

char MagCardGetPVKI (void)
{ return magCard.discretionary_data.PVKI; }

char * MagCardGetPVV (void)
{ return magCard.discretionary_data.PVV; }

char * MagCardGetCVV (void)
{ return magCard.discretionary_data.CVV; }

char MagCardGetLRC (void)
{ return magCard.LRC; }

bool MagCardIsValid (void)
{ return magCard.valid; }


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void ReadEnable (void)
{
	flag_enable = !gpioRead(PIN_MAGCARD_ENABLE);
	flag_enable_prev = !flag_enable; // Edge detection
	if (!flag_enable)
	{
		index = ss_pos_bits = fs_pos_chars = es_pos_chars = lrc_pos_chars = 0;
		flag_data_ready = flag_invalid = 0;
	}
}

static void ReadBit (void)
{
	if (flag_enable) // This could be avoided with a FSM...
	{
		track_buffer[index++] = !gpioRead(PIN_MAGCARD_DATA);
		index %= MAX_TRACK_SIZE; // Prevent writing after end of array
	}
}

static void CleanData (void)
{
	char buffer = 0;
	uint8_t i = 0;
	while (i < MAX_TRACK_SIZE && buffer != START_SENTINEL) // Search for SS
		buffer = ((buffer >> 1) & 0xF) | (track_buffer[i++] << (CHAR_LENGTH - 1));
	if (i != MAX_TRACK_SIZE)
		ss_pos_bits = i - CHAR_LENGTH;
	else
		flag_invalid = true;

	for (uint8_t i = 0; i < 40 && !flag_invalid && !lrc_pos_chars; i++) // Instead of 40 use the number of elements read
	{
		data[i] = 0;
		for (uint8_t j = 0; j < 4; j++)
			data[i] |= track_buffer[ss_pos_bits + i * 5 + j] << j; // Could clean it here instead using >>=, but LRC considers all bits...
		// parity[i] = track_buffer[i * 5]; // Could be used to check by XOR
		if (CheckRowParity(data[i], track_buffer[ss_pos_bits + i * 5 + CHAR_LENGTH])) // Parity bit
		{
			if(data[i] == FIELD_SEPARATOR)
				fs_pos_chars = i;
			else if(data[i] == END_SENTINEL)
			{
				es_pos_chars = i;
				lrc_pos_chars = i + 1;
			}
		}
		else
			flag_invalid = true; // Stop reading if data was incorrect, but may be clearer to do it at the end
	}

	flag_invalid = flag_invalid || !ss_pos_bits || !fs_pos_chars || !es_pos_chars || es_pos_chars < fs_pos_chars || fs_pos_chars < ss_pos_bits;

	magCard.LRC = data[lrc_pos_chars]; // Would not be needed if Check was done after Parse...
	magCard.valid = !flag_invalid && CheckColumnParity();
}

static void ParseData (void)
{
	magCard.data.PAN_length = fs_pos_chars - 1; // SS does not count
	uint8_t i = 1;

	for (; i < fs_pos_chars; i++)
		magCard.data.PAN[i] = data[i];

	for (i++; i < fs_pos_chars + 4; i++) // Make it clearer
		magCard.additional_data.expiration[i] = data[i];

	for (; i < fs_pos_chars + 7; i++)
		magCard.additional_data.service_code[i] = data[i];

	for (; i < fs_pos_chars + 8; i++)
		magCard.discretionary_data.PVKI = data[i];

	for (; i < fs_pos_chars + 12; i++)
		magCard.discretionary_data.PVV[i] = data[i];

	for (; i < fs_pos_chars + 15; i++)
		magCard.discretionary_data.CVV[i] = data[i];

	magCard.LRC = data[++i];
}

bool CheckRowParity (char character, bool parity) // Transversal redundancy check
{
    uint8_t count = 0;
    for (uint8_t i = 0; i < CHAR_LENGTH; i++)
        count += (character >> i) & 1;
    return count % 2 != parity; // Parity bit is 1 if even
}

bool CheckColumnParity (void) // Longitudinal redundancy check
{
    uint8_t stop = es_pos_chars / 5 - 1; // Better if this is already calculated (to use chars instead of bits)
    bool status = true;
    for (uint8_t i = 0; i < 4; i++)
    {
        uint8_t count = data[i] & 1;
        for (uint8_t j = 0; j < stop; j += 4)
            count += (data[j] >> j) & 1;
        if (count % 2 != ((magCard.LRC >> i) & 1))
            status = false;
    }
    return status; // Parity bit is 1 if even
}

// Maybe just one function that calculates parity of n bits, or maybe mxn bits
