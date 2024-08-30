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

// typedef struct {
//     uint8_t bit0_A : 1;
//     uint8_t bit1_A : 1;
//     uint8_t bit2_A : 1;
//     uint8_t bit3_A : 1;
//     uint8_t bit0_B : 1;
//     uint8_t bit1_B : 1;
//     uint8_t bit2_B : 1;
//     uint8_t bit3_B : 1;
// } MagCardData_t;

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
} MagCard_t;

// getData could return the struct or we could use getPAN, getExpiration, ... (or both?)

typedef enum {
    IDLE,
    READ,
    PARSE,
    INVALID
} MagCardState_t;


/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void ReadEnable (void);
static void ReadBit (void);
static void ReadData (void);
static void ParseData (void);


/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

// uint32_t data[5] = { 0 };
// uint8_t parity[5] = { 0 };
// MagCardData_t magCardData[20] = { 0 };
static char buffer = 0;
static char data[40] = { 0 }; // buffer?
static bool parity[40] = { 0 };
static bool track_buffer[200] = { 0 };
static uint8_t index = 1;
static bool flag_enable = false;
static bool flag_ss = false;
static bool flag_fs = false;
static bool flag_es = false;
static bool flag_invalid = false;
static const MagCard_t invalid_data = { { { 0 }, 0 }, { {0}, {0} }, {0, { 0 }, { 0 }}, 0 };
static MagCard_t magCard;
static MagCardState_t state = IDLE;
static uint8_t ss_pos = 0;
static uint8_t fs_pos = 0;
static uint8_t es_pos = 0;

// Reset function?
// Multiple function calls? Inits?


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

bool MagCardInit (void)
{
    bool status = true;

    gpioMode(PIN_MAGCARD_ENABLE, INPUT);
    gpioMode(PIN_MAGCARD_CLOCK, INPUT);
    gpioMode(PIN_MAGCARD_DATA, INPUT);

    magCard = invalid_data;

    status = gpioIRQ(PIN_MAGCARD_ENABLE, GPIO_IRQ_MODE_BOTH_EDGES, ReadEnable);
    if (status)
        status = gpioIRQ(PIN_MAGCARD_CLOCK, GPIO_IRQ_MODE_FALLING_EDGE, ReadBit);

    return status;
}

bool MagCardCheckData (void)
{
    return !flag_invalid;
}

MagCard_t MagCardGetData (void)
{
    return magCard;
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void FSM (void)
{
    switch (state)
    {
        case IDLE:
            if (flag_enable)
                state = READ;
            break;
        case READ:
            if (flag_enable)
                state = READ;
            else
                state = PARSE;
            break;
        case PARSE:
            if (flag_invalid)
                state = INVALID;
            else
                state = IDLE;
            break;
        case INVALID:
            if (flag_enable)
                state = READ;
            else
                state = INVALID;
            break;
        default:
            state = IDLE;
            break;
    }
}

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
    else // Data has ended
        ReadData();
}

static void ReadData (void)
{
    for (uint8_t i = 0; i < 40; i++)
    {
        data[i] = track_buffer[i * 5 + 4] << 0 | track_buffer[i * 5 + 3] << 1 |
                  track_buffer[i * 5 + 2] << 2 | track_buffer[i * 5 + 1] << 3;
        parity[i] = track_buffer[i * 5];

        if((flag_ss = !flag_ss && data[i] == START_SENTINEL))
            ss_pos = i;
        else if((flag_fs = !flag_fs && data[i] == FIELD_SEPARATOR))
            fs_pos = i;
        else if((flag_es = !flag_es && data[i] == END_SENTINEL))
            es_pos = i; // What if there are more than one?
        // Separate validation function?
    }
    if (!flag_ss || !flag_fs || !flag_es)
        flag_invalid = 1;
    else
    {
    }
}

static void ParseData (void)
{
    if (!flag_invalid)
    {
        // uint8_t stop = MAX_FS_POS + 1; // Current stop position to read each field
        // magCard.data.PAN_length = 0;
        uint8_t i = 4; // Skip Start Sentinel
        // while (data[i] != FIELD_SEPARATOR && i < stop) // Is it necessary or can i just use the LRC?
        for(uint8_t i = ss_pos + 1; i < fs_pos + 1; i++)
        {
            magCard.data.PAN[i] = data[i++];
            magCard.data.PAN_length++;
        }
        // for (uint8_t i = 0; i < 19 && data[i] != FIELD_SEPARATOR; i++)
        // {
        //     magCard.data.PAN[i] = data[i];
        //     magCard.data.PAN_length++;
        // }
        // if (i != stop)
        // {
        // stop += 
        for (i++; i < 4; i++)
            magCard.additional_data.expiration[i] = data[i];
        for (uint8_t i = 0; i < 3; i++)
            magCard.additional_data.service_code[i] = 0;
        magCard.discretionary_data.PVKI = 0;
        for (uint8_t i = 0; i < 4; i++)
            magCard.discretionary_data.PVV[i] = 0;
        for (uint8_t i = 0; i < 3; i++)
            magCard.discretionary_data.CVV[i] = 0;
        magCard.LRC = 0;
        // }
        // else
            // magCard = invalid_data;
    }
}

// // redo to avoid division!
// static void ReadData (void)
// {
//     if (flag_enable && !flag_invalid)
//     {
//         if (index % 5)
//             buffer |= gpioRead(PIN_MAGCARD_DATA) << (index % 5);
//         else
//         {
//             data[index / 5] = buffer;
//             parity[index / 5] = gpioRead(PIN_MAGCARD_DATA);
//         }
//         // index++ %= 40;
//         index++;

//         if (index == 40)
//             index = 1;

//         if (index == 6 && data[0] == 0xB)
//             flag_ss = true;
//         else
//             flag_invalid = true;
//         // else if (index == 40 && flag_start)
//         // {
//         //     // Check Parity
//         //     uint8_t i = 0;
//         //     for (i = 0; i < 40; i++)
//         //     {
//         //         if (parity[i] != (data[i] & 0x01))
//         //             break;
//         //     }

//         //     if (i == 40)
//         //     {
//         //         // Send Data
//         //         for (i = 0; i < 40; i++)
//         //             data[i] >>= 1;
//         //     }
//         // }
//     }
//     // TODO: check flags, use separators and not indexes (may be <19), check parity, raise flags to be checked in App with timers, ...
// }

// static char ReadChar (uint8_t bits) // 4 bits to char
// {
//     char c = 0;
//     for (uint8_t i = 0; i < 8; i++)
//     {
//         c |= gpioRead(PIN_MAGCARD_DATA) << i;
//     }
//     return c;
// }


/******************************************************************************/
