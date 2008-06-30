#ifndef _user_stdint_h
#define _user_stdint_h

// Modern versions of GCC usually have stdint.h so include it instead
#if defined(__GNUC__) && !defined(__DJGPP__)
#include <stdint.h>
#endif

// The DJGPP 
#if defined(__DJGPP__)
typedef char                int8_t;
typedef short               int16_t;
typedef int                 int32_t;
typedef long long           int64_t;

typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;
#endif

// Define specific sized variables for MSVC
#if defined(_WIN32)
typedef __int8              int8_t;
typedef __int16             int16_t;
typedef __int32             int32_t;
typedef __int64             int64_t;

typedef unsigned __int8     uint8_t;
typedef unsigned __int16    uint16_t;
typedef unsigned __int32    uint32_t;
typedef unsigned __int64    uint64_t;
#endif

#endif
