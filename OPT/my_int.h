#pragma once

typedef unsigned short u16;
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

#define RE_64(x) reinterpret_cast<ui64*>(x)
#define RE_C64(x) reinterpret_cast<const ui64*>(x)
#define RE_32(x) reinterpret_cast<ui32*>(x)
#define SC_32(x) static_cast<ui32*>(x)

#if defined(__IBMC__) || defined(__IBMCPP__)
#define nullptr 0
#endif