/****************************************************************************

 i106_index.h - 

 Copyright (c) 2006 Irig106.org

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

/** @file
IRIG 106 file index handling

*/


#ifndef _I106_INDEX_H
#define _I106_INDEX_H

#ifdef __cplusplus
namespace Irig106 {
extern "C" {
#endif

/*
 * Macros and definitions
 * ----------------------
 */


/*
 * Data structures
 * ---------------
 */

typedef struct
    {
    uint16_t        uChID;              ///< Channel ID
    uint8_t         ubyDataType;        ///< Data type
    int64_t         lRelTime;           ///< 48 bit relative time
    SuIrig106Time   suIrigTime;         ///< Absolute time
    int64_t         lFileOffset;        ///< File offset to packet
    } SuPacketIndexInfo;


/*
 * Global data
 * -----------
 */


/*
 * Function Declaration
 * --------------------
 */

/** Initialize index data structures
    @param iHandle      Handle to an open IRIG 106 data stream
*/
void InitIndex(int iHandle);

/** Sort the in-memory index by RTC value
    @param iHandle      Handle to an open IRIG 106 data stream
*/
void SortIndexes(int iHandle);

/** Determine if an index is present. 
    @param iHandle      Handle of an IRIG file already opened for reading
    @param bFoundIndex  True if index found
    @return             I106_OK if return value valid
*/
EnI106Status I106_CALL_DECL enIndexPresent(const int iHandle, int * bFoundIndex);

/** Read an open Ch 10 file, read the various index packets, and build an in-memory table of time and offsets.
    @param iHandle      Handle of an IRIG file already opened for reading
    @return             I106_OK if index data valid
*/
EnI106Status I106_CALL_DECL enReadIndexes(const int iHandle);

/** Make an index of a channel by reading through the data file.
    @param iHandle      Handle of an IRIG file already opened for reading
    @param uChID        Channel ID of channel to index
    @return             I106_OK if return value valid
*/
EnI106Status I106_CALL_DECL enMakeIndex(const int iHandle, uint16_t uChID);

//EnI106Status I106_CALL_DECL SaveIndexTable(char* strFileName);


/** Add index information to the in memory index array
    @param iHandle      Handle of an IRIG file already opened for reading
    @param psuIndexInfo Structure to hold the packet index info
*/
void AddNodeToIndex(int iHandle, SuPacketIndexInfo * psuIndexInfo);

/** Get a pointer to the array of SuPacketIndexInfo and thelength of the array 
    @param iHandle              Handle of an IRIG file 
    @param asuPacketIndexInfo   Array of structures that hold the packet index info
    @param piArrayLength        Variable pointer that return array length
    @return                     I106_OK if return values valid
*/
EnI106Status I106_CALL_DECL enGetIndexArray(const int iHandle, SuPacketIndexInfo * asuPacketIndexInfo[], uint32_t * piArrayLength);


#ifdef __cplusplus
} // end extern "C"
} // end namespace
#endif

#endif

