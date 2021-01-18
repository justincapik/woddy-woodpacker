#include "wwp.h"

u_int16_t	u16s(u_int16_t val) 
{
    return ((val << 8) | (val >> 8));
}

u_int32_t	u32s(u_int32_t n)
{
	n = ((n << 8) & 0xFF00FF00) | ((n >> 8) & 0x00FF00FF);
	return ((n << 16) | (n >> 16));
}

u_int64_t	u64s(u_int64_t n)
{
	n = ((n << 8) & 0xFF00FF00FF00FF00ULL) \
		| ((n >> 8) & 0x00FF00FF00FF00FFULL);
	n = ((n << 16) & 0xFFFF0000FFFF0000ULL) \
		| ((n >> 16) & 0x0000FFFF0000FFFFULL);
	return ((n << 32) | (n >> 32));
}
