#ifndef _stdint_h
#define _stdint_h

// Modern versions of GCC usually have stdint.h so include it instead
#if defined(__GNUC__)
#include <stdint.h>
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
