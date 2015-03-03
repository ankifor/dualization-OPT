#pragma once

typedef unsigned int ui32;
typedef signed int i32;
typedef unsigned long long ui64;

#define UI32_BITS ui32(32)
#define UI32_LOG2BIT ui32(5)
#define UI32_MASK  ui32(31)
#define UI32_SIZE ui32(4)
#define UI32_ALL (~ui32(0))
#define UI32_SIGN (0x80000000)

#define UI64_LOG2BIT ui32(6)
#define UI64_MASK  ui32(63)
#define UI64_BITS ui32(64)
#define UI64_SIZE ui32(8)
#define UI64_ALL (~ui64(0))