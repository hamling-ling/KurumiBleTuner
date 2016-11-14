#ifndef _OSAKANAFFTUTIL_H_
#define _OSAKANAFFTUTIL_H_

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static inline uint32_t bitSwap(uint32_t x, int a, int b)
{
	uint32_t bita = (1 << a);
	uint32_t bitb = (1 << b);
	uint32_t out = x & ~(bita | bitb);// remove bit a and b

	// swap position
	bita = (bita&x) << (b - a);
	bitb = (bitb&x) >> (b - a);
	// assign bit
	out |= (bita | bitb);
	return out;
}

static inline uint32_t bitReverse(int log2N, uint32_t x)
{
	uint32_t out = x;
	for (int i = 0; i < log2N / 2; i++) {
		out = bitSwap(out, i, log2N - 1 - i);
	}
	return out;

}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
