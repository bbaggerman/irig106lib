

#ifndef _I106_DECODE_TIME_H
#define _I106_DECODE_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push,1)


/*
 * Macros and definitions
 * ----------------------
 */

typedef enum
    {
    I106_TIMEFMT_IRIG_B      =  0x00,
    I106_TIMEFMT_IRIG_A      =  0x01,
    I106_TIMEFMT_IRIG_G      =  0x02,
    I106_TIMEFMT_INT_RTC     =  0x03,
    I106_TIMEFMT_GPS_UTC     =  0x04,
    I106_TIMEFMT_GPS_NATIVE  =  0x05,
    } EnI106TimeFmt;


/*
 * Data structures
 * ---------------
 */

/* Time Format 1 */

/* Time */
/*
typedef struct
    {
    uint16_t     bCarrier    : 1;        // IRIG time carrier present
    uint16_t     uUnused1    : 3;        // Reserved
    uint16_t     uFormat     : 2;        // IRIG time format
    uint16_t     uUnused2    : 2;        // Reserved
    uint16_t     bLeapYear   : 1;        // Leap year
    uint16_t     uUnused3    : 7;        // Reserved
    uint16_t     uUnused4;
    uint16_t     uUnused5;
    } SuIrigTime;
*/


// Decoded time

// The nice thing about standards is that there are so many to
// choose from, and time is no exception. But none of the various
// C time representations really fill the bill. So I made a new
// time representation.  So there.
typedef struct 
    {
    unsigned long        ulSecs;    // This is a time_t
    unsigned long        ulFrac;    // LSB = 100ns
    } SuIrigTimeF1;

/*
 * Function Declaration
 * --------------------
 */

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106_Decode_TimeF1(SuI106Ch10Header  * psuHeader,
                         void              * pvBuff,
                         SuIrigTimeF1      * psuTime);

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106_Rel2IrigTime(uint8_t            abyRelTime[],
                        SuIrigTimeF1     * psuTime);


//void iInit_DOY2DMY(int iYear);

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif