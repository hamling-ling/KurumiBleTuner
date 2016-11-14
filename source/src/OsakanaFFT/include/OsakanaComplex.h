#ifndef _OSAKANACOMPLEX_H_
#define _OSAKANACOMPLEX_H_

#ifdef __MBED__
#include <mbed.h>
#else
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct {
	float re;
	float im;
} osk_complex_t;

static inline osk_complex_t MakeComplex(float re, float im)
{
	osk_complex_t complex;
	complex.re = re;
	complex.im = im;
	return complex;
}

static inline osk_complex_t complex_add(const osk_complex_t* a, const osk_complex_t* b)
{
	osk_complex_t x;
	x.re = a->re + b->re;
	x.im = a->im + b->im;
	return x;
}

static inline osk_complex_t complex_sub(const osk_complex_t* a, const osk_complex_t* b)
{
	osk_complex_t x;
	x.re = a->re - b->re;
	x.im = a->im - b->im;
	return x;
}

static inline osk_complex_t complex_mult(const osk_complex_t* a, const osk_complex_t* b)
{
	osk_complex_t x;
	x.re = a->re * b->re - a->im * b->im;
	x.im = a->re * b->im + a->im * b->re;
	return x;
}

static inline void complex_swap(osk_complex_t* a, osk_complex_t* b)
{
	osk_complex_t temp = *a;
	a->re = b->re;
	a->im = b->im;
	*b = temp;
}

static inline char* complex_str(const osk_complex_t* a, char* buf, size_t buf_size)
{
	snprintf(buf, buf_size, "%f, %f", a->re, a->im);

	return buf;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
