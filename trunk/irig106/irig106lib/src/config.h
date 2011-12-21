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

 ****************************************************************************/

#ifndef _config_h_
#define _config_h_

#ifdef __cplusplus
extern "C" {
#endif

// .NET 2005 C++ wants structures that are passed as function parameters to be declared
// as public.  .NET 2003 and native C pukes on that. C++ Interop doesn't seem to care.
//  Grrrr...  Just define out PUBLIC for now but leave in the macro logic in case I want
// to revisit this someday.  Yeah, right!
//#if _MSC_VER >= 1400
//#define PUBLIC public
//#else
#define PUBLIC
//#endif

// .NET 2005 (and probably earlier, but I'm not sure) define time_t to be a 64 bit value.
// And by default, all the CRT time routines are the 64 bit versions.  For best portability,
// time_t is assumed to be a 32 bit value.  The following #define tells .NET to use 32 bits
// as the default time_t size.  This needs to be set in the project properties.  This forces
// a puke if it isn't set.
#if _MSC_VER >= 1400
  #if !defined(_USE_32BIT_TIME_T)
  #pragma message("WARNING - '_USE_32BIT_TIME_T' not set!")
  #endif
#endif

// .NET managed code extends good ol' Stroustrup C++ in some interesting and unique ways.
// I don't know what Bjarne would say, but here in the real world we need to deal with it.
#if defined(_M_CEE)
#define PUBLIC_CLASS    public

#else
#define PUBLIC_CLASS

#endif

/* The POSIX caseless string compare is strcasecmp(). MSVC uses the
 * non-standard stricmp(). Fix it up with a macro if necessary
 */

#if defined(_MSC_VER) 
#define strcasecmp(s1, s2)          _stricmp(s1, s2)
#define strncasecmp(s1, s2, n)      _strnicmp(s1, s2, n)
#pragma warning(disable : 4996)
#endif

#define I106_CALL_DECL

// Turn on network support
// #define IRIG_NETWORKING

#ifdef __cplusplus
}
#endif

#endif




