/****************************************************************************

 config.h - Define features and OS portability macros

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

 Created by Bob Baggerman

 $RCSfile: config.h,v $
 $Date: 2006-11-20 04:43:24 $
 $Revision: 1.1 $

 ****************************************************************************/

#ifndef _config_h_
#define _config_h_

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Macros and definitions
 * ----------------------
 */

/* The POSIX caseless string compare is strcasecmp(). MSVC uses the
 * non-standard stricmp(). Fix it up with a macro if necessary
 */

#if defined(_MSC_VER) 
#define strcasecmp(s1, s2)          stricmp(s1, s2)
#define strncasecmp(s1, s2, n)      strnicmp(s1, s2, n)
#endif

/*
 * Data structures
 * ---------------
 */


/*
 * Global data
 * -----------
 */


/*
 * Function Declaration
 * --------------------
 */


 
#ifdef __cplusplus
}
#endif

#endif



