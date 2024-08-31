/***************************************************************************//**
  @file     magcard.c
  @brief    Magnetic Stripe Card Reader driver
  @author   Grupo 4
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "magcard.h"
#include "gpio.h"
#include "SysTick.h"
#include "board.h"
#include <stdbool.h>
#include <stdint.h>


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define DEVELOPMENT_MODE    1
#define MAX_TRACK_SIZE      200
#define START_SENTINEL      0xB
#define FIELD_SEPARATOR     0xD
#define END_SENTINEL        0xF
#define INVALID_DATA        { 0 }
#define CHAR_LENGTH         4
#define MAX_PAN_LENGTH      19
#define MAX_FS_POS          1 + MAX_PAN_LENGTH


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
static bool CheckLRC (void);
static void ParseData (void);


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static char data[40] = { 0 }; // There has to be a better option...
static bool parity[40] = { 0 };
static bool track_buffer[200] = { 0 };
static uint8_t index = 1;

// static bool data[40][4] = { { 0 } }; // Would use 1 byte to store 1 bit!!! BUt is convenient...

// MagCardFlags_t flags = { 0 };
static bool flag_init = false;
static bool flag_enable = false;
static bool flag_sort = false;
static bool flag_parity = false;
static bool flag_parse = false;
static bool flag_invalid = false;

static uint8_t ss_pos = 0;
static uint8_t fs_pos = 0;
static uint8_t es_pos = 0;

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

    if (flag_init != true) // NOt initialized yet
    {
        status = flag_init = true;
        magCard.valid = false;

        gpioMode(PIN_MAGCARD_ENABLE, INPUT);
        gpioMode(PIN_MAGCARD_CLOCK, INPUT);
        gpioMode(PIN_MAGCARD_DATA, INPUT);

        if (status)
            status = !gpioIRQ(PIN_MAGCARD_ENABLE, GPIO_IRQ_MODE_BOTH_EDGES, ReadEnable); // Periodic?
        if (status)
            status = !gpioIRQ(PIN_MAGCARD_CLOCK, GPIO_IRQ_MODE_FALLING_EDGE, ReadBit);
    }

    return status; // should return 1 if there was an error
}

bool MagCardCheckData (void) // Needed? Or return empty data in MagCardGetData (and call it MagCardRead)?
{
    return flag_init && magCard.valid; // magCard may not be initialized
}

MagCard_t MagCardGetData (void)
{
    if (!flag_init)
        magCard.valid = false;

    return magCard;
}

// MagCard Fields

MagCardData_t MagCardGetPANData (void)
{ return magCard.data; }

MagCardAdditionalData_t MagCardGetAdditionalData (void)
{ return magCard.additional_data; }

MagCardDiscretionaryData_t MagCardGetDiscretionaryData (void)
{ return magCard.discretionary_data; }

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

// static void FSM (void)
// {
//     switch (state)
//     {
//         case IDLE:
//             if (flag_enable)
//                 state = READ;
//             break;

//         case READ:
//             if (flag_enable)
//             {
//                 state = READ;
//                 ReadBit();
//             }
//             else
//                 state = SORT;
//             break;

//         case SORT:
//             if (flag_enable)
//             {
//                 state = SORT;
//                 SortData();
//             }
//             else
//                 state = PARSE;
//             break;

//         case PARSE:
//             if (flag_invalid)
//                 state = INVALID;
//             else
//                 state = IDLE;
//             break;

//         case INVALID:
//             if (flag_enable)
//                 state = READ;
//             else
//                 state = INVALID;
//             break;

//         default:
//             state = IDLE;
//             break;
//     }
// }

static void ReadEnable (void)
{
    flag_enable = !gpioRead(PIN_MAGCARD_ENABLE);
}

static void ReadBit (void)
{
    if (flag_enable)
    {
        track_buffer[index++] = gpioRead(PIN_MAGCARD_DATA);
        index %= MAX_TRACK_SIZE;
    }
    else // Data has ended, but would be better to check falling edge or do it outside the interrupt
    {
        CleanData();
        index = 0;
    }
}

static void CleanData (void)
{
    if (!flag_sort)
    {
        for (uint8_t i = 0; i < 40 && !flag_invalid; i++)
        {
            data[i] = 0;
            for (uint8_t j = 1; j < 5; j++)
                data[i] |= track_buffer[i * 5 + j] << (CHAR_LENGTH - j); // Could clean it here instead using >>=, but LRC considers all bits...
            // parity[i] = track_buffer[i * 5];
            if (CheckRowParity(data[i], track_buffer[i * 5])) // Parity bit
            {
                if(data[i] == START_SENTINEL) // Maybe better in Parse...
                    ss_pos = i;
                else if(data[i] == FIELD_SEPARATOR)
                    fs_pos = i;
                else if(data[i] == END_SENTINEL)
                    es_pos = i;
            }
            else
                flag_invalid = true;
        }

        flag_sort = true;
        if (CheckColumnParity() && !flag_invalid) // This should not be in an interrupt
            magCard.valid = false;
        else
        {
            magCard.valid = true;
            ParseData(); // This neither
        }
    }
}

static void ParseData (void)
{
    if (!flag_parse)
    {
        magCard.data.PAN_length = fs_pos - ss_pos;
        uint8_t i = ss_pos + 1;
        // while (data[i] != FIELD_SEPARATOR && i < stop)

        for (; i < fs_pos; i++)
            magCard.data.PAN[i] = data[i];

        for (i++; i < fs_pos + 4; i++) // Make it clearer
            magCard.additional_data.expiration[i] = data[i];

        for (; i < fs_pos + 7; i++)
            magCard.additional_data.service_code[i] = data[i];

        for (; i < fs_pos + 8; i++)
            magCard.discretionary_data.PVKI = data[i];
        
        for (; i < fs_pos + 12; i++)
            magCard.discretionary_data.PVV[i] = data[i];
        
        for (; i < fs_pos + 15; i++)
            magCard.discretionary_data.CVV[i] = data[i];

        magCard.LRC = data[++i];

        flag_parse = true;
    }
}

// TODO: check flags, use separators and not indexes (may be <19), check parity, raise flags to be checked in App with timers, ...

bool CheckRowParity (char character, bool parity) // Transversal redundancy check
{
    uint8_t count = character & 1;
    for (uint8_t i = 0; i < CHAR_LENGTH; i++)
        count += (character >> i) & 1;
    return count % 2 != parity; // parity bit is 1 if odd
}

bool CheckColumnParity (void) // Longitudinal redundancy check
{
    uint8_t stop = es_pos / 5 - 1; // Better if this is already calculated (to use chars instead of bits)
    bool status = true;
    for (uint8_t i = 0; i < 4; i++)
    {
        uint8_t count = data[i] & 1;
        for (uint8_t j = 0; i < stop; j += 4)
            count += (data[j] >> j) & 1;
        if (count % 2 != (magCard.LRC >> i) & 1)
            status = false;
    }
    return status; // parity bit is 1 if odd
}

// Maybe just one function that calculates parity of n bits

/******************************************************************************/
