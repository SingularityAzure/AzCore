/*
	File: Util.hpp
	Author: Philip Haynes
	Memory utility functions
*/

#ifndef AZCORE_MEMORY_UTIL_HPP
#define AZCORE_MEMORY_UTIL_HPP

#include "../basictypes.hpp"
#include "string.h"
#include <memory>

#if defined(__GNUG__)
#elif defined(_MSC_VER)
	#include <intrin.h>
#endif

namespace AzCore {

template<typename T>
void Swap(T &a, T &b) {
	T c = std::move(a);
	a = std::move(b);
	b = std::move(c);
}

constexpr bool IsPowerOfTwo(size_t value) {
	return (value & (value-1)) == 0;
}
size_t align(size_t size, size_t alignment);
size_t alignNonPowerOfTwo(size_t size, size_t alignment);

inline u32 CountTrailingZeroBits(u32 value) {
	#if defined(__GNUG__)
		u32 result;
		// We could use __builtin_ctz but it throws away the check we get from bsf for the cmov
		asm (
			"bsf %0, %1\n\t"
			"cmovz %0, $32"
			: "=r" (result)
			: "r" (value)
		);
		return result;
	#elif defined(_MSC_VER)
		unsigned long result;
		return _BitScanForward(&result, value) ? result : 32;
	#endif
}

// Extract the base 2 exponent directly from the bits.
#if AZCORE_COMPILER_SUPPORTS_128BIT_TYPES
force_inline(i16)
Exponent(f128 value) {
	u128 byteCode;
	memcpy((void *)&byteCode, (void *)&value, sizeof(byteCode));
	i16 exponent = ((byteCode >> 112) & 0x7fff) - 0x3fff;
	return exponent;
}
#endif

// Extract the base 2 exponent directly from the bits.
force_inline(i16)
Exponent(f64 value) {
	u64 byteCode;
	memcpy((void *)&byteCode, (void *)&value, sizeof(byteCode));
	i16 exponent = ((i16)(byteCode >> 52) & 0x7ff) - 0x3ff;
	return exponent;
}

// Extract the base 2 exponent directly from the bits.
force_inline(i16)
Exponent(f32 value) {
	u32 byteCode;
	memcpy((void *)&byteCode, (void *)&value, sizeof(byteCode));
	i16 exponent = ((i16)(byteCode >> 23) & 0xff) - 0x7f;
	return exponent;
}

} // namespace AzCore

#endif // AZCORE_MEMORY_UTIL_HPP