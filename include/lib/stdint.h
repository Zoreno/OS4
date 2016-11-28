#ifndef _STDINT_H
#define _STDINT_H

#include "size_t.h"

//===================================================================
// Exact width integer types
//===================================================================

typedef signed char 			int8_t;
typedef unsigned char 			uint8_t;
typedef signed short 			int16_t;
typedef unsigned short 			uint16_t;
typedef signed int 				int32_t;
typedef unsigned int 			uint32_t;
typedef signed long long 		int64_t;
typedef unsigned long long 		uint64_t;

#define INT8_MIN (-128) 
#define INT16_MIN (-32768)
#define INT32_MIN (-2147483647 - 1)
#define INT64_MIN  (-9223372036854775807LL - 1)

#define INT8_MAX 127
#define INT16_MAX 32767
#define INT32_MAX 2147483647
#define INT64_MAX 9223372036854775807LL

#define UINT8_MAX 0xff
#define UINT16_MAX 0xffff
#define UINT32_MAX 0xffffffff
#define UINT64_MAX 0xffffffffffffffffULL

//===================================================================
// Minimum width integer types
//===================================================================

typedef signed char 			int_least8_t;
typedef unsigned char   		uint_least8_t;
typedef short  					int_least16_t;
typedef unsigned short  		uint_least16_t;
typedef int  					int_least32_t;
typedef unsigned   				uint_least32_t;
typedef long long  				int_least64_t;
typedef unsigned long long   	uint_least64_t;

#define INT_LEAST8_MIN INT8_MIN
#define INT_LEAST16_MIN INT16_MIN
#define INT_LEAST32_MIN INT32_MIN
#define INT_LEAST64_MIN INT64_MIN

#define INT_LEAST8_MAX INT8_MAX
#define INT_LEAST16_MAX INT16_MAX
#define INT_LEAST32_MAX INT32_MAX
#define INT_LEAST64_MAX INT64_MAX

#define UINT_LEAST8_MAX UINT8_MAX
#define UINT_LEAST16_MAX UINT16_MAX
#define UINT_LEAST32_MAX UINT32_MAX
#define UINT_LEAST64_MAX UINT64_MAX

#define INT8_C(val) ((int8_t) + (val))
#define UINT8_C(val) ((uint8_t) + (val##U))
#define INT16_C(val) ((int16_t) + (val))
#define UINT16_C(val) ((uint16_t) + (val##U))

#define INT32_C(val) val##L
#define UINT32_C(val) val##UL
#define INT64_C(val) val##LL
#define UINT64_C(val) val##ULL

//===================================================================
// Fastest minimum width integer types
//===================================================================

typedef char 					int_fast8_t;
typedef unsigned char 			uint_fast8_t;
typedef short  					int_fast16_t;
typedef unsigned short  		uint_fast16_t;
typedef int  					int_fast32_t;
typedef unsigned  int  			uint_fast32_t;
typedef long long  				int_fast64_t;
typedef unsigned long long   	uint_fast64_t;

#define INT_FAST8_MIN INT8_MIN
#define INT_FAST16_MIN INT16_MIN
#define INT_FAST32_MIN INT32_MIN
#define INT_FAST64_MIN INT64_MIN

#define INT_FAST8_MAX INT8_MAX
#define INT_FAST16_MAX INT16_MAX
#define INT_FAST32_MAX INT32_MAX
#define INT_FAST64_MAX INT64_MAX

#define UINT_FAST8_MAX UINT8_MAX
#define UINT_FAST16_MAX UINT16_MAX
#define UINT_FAST32_MAX UINT32_MAX
#define UINT_FAST64_MAX UINT64_MAX

//===================================================================
// Pointer types
//===================================================================

typedef int 					intptr_t;
typedef unsigned 				uintptr_t;

#define INTPTR_MIN INT32_MIN
#define INTPTR_MAX INT32_MAX

#define UINTPTR_MAX UINT32_MAX

//===================================================================
// Greatest width integer types
//===================================================================

typedef long long  				intmax_t;
typedef unsigned long long   	uintmax_t;

#define INTMAX_MIN INT64_MIN
#define INTMAX_MAX INT64_MAX

#define UINTMAX_MAX UINT64_MAX

#define INTMAX_C(val)  INT64_C(val)
#define UINTMAX_C(val) UINT64_C(val)

//===================================================================
// Other types
//===================================================================

#define PTRDIFF_MIN INT32_MIN
#define PTRDIFF_MAX INT32_MAX

#define SIG_ATOMIC_MIN INT32_MIN
#define SIG_ATOMIC_MAX INT32_MAX

#define SIZE_MAX UINT32_MAX

#define	LONG_MIN	(-2147483647L-1L)	/* min value of a "long int" */
#define	LONG_MAX	2147483647L			/* max value of a "long int" */
#define	ULONG_MAX	4294967295UL 		/* max value of "unsigned long int" */

#endif
