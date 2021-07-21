/****************************************************************************

 i106_decode_tmats_common.h - Decode TMATS common macros and data structures

 Copyright (c) 2018 Irig106.org

 All rights reserved.

 Redistribution and use in source and binary forms, with or without 
 modification, are permitted provided that the following conditions are 
 met:

   * Redistributions of source code must retain the above copyright 
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright 
     notice, this list of conditions and the following disclaimer in the 
     documentation and/or other materials provided with the distribution.

   * Neither the name Irig106.org nor the names of its contributors may 
     be used to endorse or promote products derived from this software 
     without specific prior written permission.

 This software is provided by the copyright holders and contributors 
 "as is" and any express or implied warranties, including, but not 
 limited to, the implied warranties of merchantability and fitness for 
 a particular purpose are disclaimed. In no event shall the copyright 
 owner or contributors be liable for any direct, indirect, incidental, 
 special, exemplary, or consequential damages (including, but not 
 limited to, procurement of substitute goods or services; loss of use, 
 data, or profits; or business interruption) however caused and on any 
 theory of liability, whether in contract, strict liability, or tort 
 (including negligence or otherwise) arising in any way out of the use 
 of this software, even if advised of the possibility of such damage.

 ****************************************************************************/

#ifndef _I106_DECODE_TMATS_COMMON_H
#define _I106_DECODE_TMATS_COMMON_H

#ifdef __cplusplus
namespace Irig106 {
extern "C" {
#endif

/*
 * Macros and definitions
 * ----------------------
 */

// Macro to make linked list search function
// -----------------------------------------

/* 
This macro is used to implement a search function for a linked list. The TMATS index number
is passed in as the search parameter. The function either finds the record in the list with
the matching index number, or may create a new one at the end.
For a linked list, create the search function with a line like the following:

    MAKE_GetRecordByIndex(record_type, record_typedef)

where 
    "record_type" is a unique identifier for this linked list
    "record_typedef" is the actual struct typedef for the linked list

The macro will create a function:

record_typedef * psuGet##record_type##ByIndex(record_typedef ** ppsuFirstRecord, int iIndex, int bMakeNew)

where
    "ppsuFirstRecord" is a pointer to the first record in the linked list
    "iIndex" is the index number to search for
    "bMakeNew" is a flag indicating whether not to add a new record if a matching one isn't found

Example:

    MAKE_GetRecordByIndex(RRecord, SuRRecord)

This will create a search function of the form:

    SuRRecord * psuGetRRecordByIndex(SuRRecord ** ppsuFirstRRecord, int iRIndex, int bMakeNew)

*/

#define MAKE_GetRecordByIndex(record_typedef)                                                  \
record_typedef * psuGetRecordByIndex_##record_typedef(record_typedef ** ppsuFirstRecord, int iIndex, int bMakeNew)  \
    {                                                                                           \
    record_typedef   ** ppsuCurrRec = ppsuFirstRecord;                                          \
                                                                                                \
    /* Loop looking for matching index number or end of list */                                 \
    while (bTRUE)                                                                               \
        {                                                                                       \
        /* Check for end of list */                                                             \
        if (*ppsuCurrRec == NULL)                                                               \
            break;                                                                              \
                                                                                                \
        /* Check for matching index number */                                                   \
        if ((*ppsuCurrRec)->iIndex == iIndex)                                                   \
            break;                                                                              \
                                                                                                \
        /* Move on to the next record in the list */                                            \
        ppsuCurrRec = &((*ppsuCurrRec)->psuNext);                                               \
        }                                                                                       \
                                                                                                \
    /* If no record found then put a new one on the end of the list */                          \
    if ((*ppsuCurrRec == NULL) && (bMakeNew == bTRUE))                                          \
        {                                                                                       \
        /* Allocate memory for the new record */                                                \
        *ppsuCurrRec = (record_typedef *)TmatsMalloc(sizeof(**ppsuCurrRec));                    \
        memset(*ppsuCurrRec, 0, sizeof(**ppsuCurrRec));                                         \
        (*ppsuCurrRec)->iIndex = iIndex;                                                        \
        } /* end if new record */                                                               \
                                                                                                \
    return *ppsuCurrRec;                                                                        \
    } /* end psuGetRRecord() */


// Iterate over a linked list
// --------------------------

/*
Iterate over a linked list. The list item is in the "psuRecord" variable. This assumes that
the list has a "psuNext" variable to point to the next list item.
*/

#define FOREACH(record_typedef, first_record)                                                   \
    for (record_typedef * psuRecord=first_record; psuRecord!=NULL; psuRecord=psuRecord->psuNext)


// Macros to make decoding record logic more compact
// -------------------------------------------------

/*
For these macros to work some variables need to be defined and some up front code needs
to be run. Here is an example of the required stuff.

    int             iTokens;
    int             iRecordIndex;
    char            szCodeName2[2000];
    char            szCodeField[2000];
    char          * pcChar;
    int             iIndex1, iIndex2, iIndex3, iIndex4;
    SuRecord      * psuRec;


    // Parse to get the record index number, the record, and the rest of the line
    iTokens = sscanf(szCodeName, "%*1c-%i\\%s", &iRecordIndex, szCodeName2);
    if (iTokens == 2)
        {
        // Get the current working record based in the index just read
        psuRecord = psuGetRecordByIndex(ppsuFirstRecord, iRecordIndex, bTRUE);
        }
    else
        return 1;

    // Break the rest of the line apart for further analysis
    for (pcChar=szCodeName2; *pcChar!='\0'; pcChar++)
        if (*pcChar == '-') *pcChar = ' ';
    iTokens = sscanf(szCodeName2, "%s %d %d %d %d", szCodeField, &iIndex1, &iIndex2, &iIndex3, &iIndex4);

Example:
    szCodeName  = "R-2\VLNB-5-7"
    iIndex      = 2
    szCodeName2 = "VLNB-5-7"
    iTokens     = 3
    szCodeField = "VLNB"
    iIndex1     = 5
    iIndex2     = 7
    iIndex3     = undefined
    iIndex4     = undefined

*/

// Decode and store a line with no iterator (i.e. "XXX")
#define DECODE(pattern, field)                                                  \
    if (strcasecmp(szCodeField, #pattern) == 0)                                 \
        if (iTokens == 1)                                                       \
            {                                                                   \
            field = szDataItem;                                                 \
            return 0;                                                           \
            }
        
// Decode and store a line with 1 iterator (i.e. "XXX-n")
// "base_record_typedef" needs to match one of "GetRecordByIndex" search types defined by using the above macro
#define DECODE_1(pattern, field, base_record_first, base_record_typedef)        \
    if (strcasecmp(szCodeField, #pattern) == 0)                                 \
        {                                                                       \
        base_record_typedef     * psuCurr;                                      \
        if (iTokens == 2)                                                       \
            {                                                                   \
            psuCurr = psuGetRecordByIndex_##base_record_typedef(&(base_record_first), iIndex1, bTRUE); \
            assert(psuCurr != NULL);                                            \
            psuCurr->field = szDataItem;                                        \
            return 0;                                                           \
            } /* end if sscanf OK */                                            \
        } /* end if pattern found */

// Decode and store a line with 2 iterators (i.e. "XXX-i-n")
#define DECODE_2(pattern, field, base_record_first, base_record_typedef, record_2_first, record_2_typedef) \
    if (strcasecmp(szCodeField, #pattern) == 0)                                 \
        {                                                                       \
        base_record_typedef     * psuCurr1;                                     \
        record_2_typedef        * psuCurr2;                                     \
        if (iTokens == 3)                                                       \
            {                                                                   \
            psuCurr1 = psuGetRecordByIndex_##base_record_typedef(&(base_record_first), iIndex1, bTRUE);     \
            assert(psuCurr1 != NULL);                                                                       \
            psuCurr2 = psuGetRecordByIndex_##record_2_typedef(&(psuCurr1->record_2_first), iIndex2, bTRUE); \
            psuCurr2->field = szDataItem;                                                                   \
            return 0;                                                           \
            }                                                                   \
        } /* end if pattern found */

// Decode and store a line with 3 iterators (i.e. "XXX-i-n-p")
#define DECODE_3(pattern, field, base_record_first, base_record_typedef,        \
                                 record_2_first,    record_2_typedef,           \
                                 record_3_first,    record_3_typedef)           \
    if (strcasecmp(szCodeField, #pattern) == 0)                                 \
        {                                                                       \
        base_record_typedef     * psuCurr1;                                     \
        record_2_typedef        * psuCurr2;                                     \
        record_3_typedef        * psuCurr3;                                     \
        if (iTokens == 4)                                                       \
            {                                                                   \
            psuCurr1 = psuGetRecordByIndex_##base_record_typedef(&(base_record_first), iIndex1, bTRUE);     \
            assert(psuCurr1 != NULL);                                                                       \
            psuCurr2 = psuGetRecordByIndex_##record_2_typedef(&(psuCurr1->record_2_first), iIndex2, bTRUE); \
            assert(psuCurr2 != NULL);                                                                       \
            psuCurr3 = psuGetRecordByIndex_##record_3_typedef(&(psuCurr2->record_3_first), iIndex3, bTRUE); \
            assert(psuCurr3 != NULL);                                                                       \
            psuCurr3->field = szDataItem;                                                                   \
            return 0;                                                           \
            }                                                                   \
        } /* end if pattern found */

// Decode and store a line with 4 iterators (i.e. "XXX-i-n-p-e")
#define DECODE_4(pattern, field, base_record_first, base_record_typedef,        \
                                 record_2_first,    record_2_typedef,           \
                                 record_3_first,    record_3_typedef,           \
                                 record_4_first,    record_4_typedef)           \
    if (strcasecmp(szCodeField, #pattern) == 0)                                 \
        {                                                                       \
        base_record_typedef     * psuCurr1;                                     \
        record_2_typedef        * psuCurr2;                                     \
        record_3_typedef        * psuCurr3;                                     \
        record_4_typedef        * psuCurr4;                                     \
        if (iTokens == 5)                                                       \
            {                                                                   \
            psuCurr1 = psuGetRecordByIndex_##base_record_typedef(&(base_record_first), iIndex1, bTRUE);     \
            assert(psuCurr1 != NULL);                                                                       \
            psuCurr2 = psuGetRecordByIndex_##record_2_typedef(&(psuCurr1->record_2_first), iIndex2, bTRUE); \
            assert(psuCurr2 != NULL);                                                                       \
            psuCurr3 = psuGetRecordByIndex_##record_3_typedef(&(psuCurr2->record_3_first), iIndex3, bTRUE); \
            assert(psuCurr3 != NULL);                                                                       \
            psuCurr4 = psuGetRecordByIndex_##record_4_typedef(&(psuCurr3->record_4_first), iIndex4, bTRUE); \
            assert(psuCurr4 != NULL);                                                                       \
            psuCurr4->field = szDataItem;                                                                   \
            return 0;                                                           \
            }                                                                   \
        } /* end if pattern found */

// Decode and store a comment line with 1 iterator (i.e. "x\COM-n")
// "base_record_typedef" needs to match one of "GetRecordByIndex" search types defined by using the above macro
#define DECODE_COMMENT(comment, base_record_first, base_record_typedef)         \
    if (strcasecmp(szCodeField, "COM") == 0)                                    \
        {                                                                       \
        base_record_typedef     * psuCurr;                                      \
        if (iTokens == 2)                                                       \
            {                                                                   \
            psuCurr = psuGetRecordByIndex_##base_record_typedef(&(base_record_first), iIndex1, bTRUE); \
            assert(psuCurr != NULL);                                            \
            StoreComment(comment, &(psuCurr->psuFirstComment));                 \
            return 0;                                                           \
            } /* end if sscanf OK */                                            \
        } /* end if pattern found */

/*
 * Data structures
 * ---------------
 */

// Comment
typedef PUBLIC struct SuComment_S
    {
    char                       * szComment;
    struct SuComment_S         * psuNext;
    } SuComment;


// Point of contact
typedef PUBLIC struct SuPointOfContact_S
    {
    int                          iIndex;                 // X\POC-n
    char                       * szName;                 // X\POC1-n
    char                       * szAgency;               // X\POC2-n
    char                       * szAddress;              // X\POC3-n
    char                       * szTelephone;            // X\POC4-n
    struct SuPointOfContact_S  * psuNext;
    } SuPointOfContact;


// Function Prototypes
// -------------------

void * TmatsMalloc(size_t iSize);

#ifdef __cplusplus
}
}
#endif

#endif
