#ifndef _OSAKANACOMPLEXFP_H_
#define _OSAKANACOMPLEXFP_H_

#include "OsakanaFp.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct {
	Fp_t re;
	Fp_t im;
} osk_fp_complex_t;

static inline osk_fp_complex_t FpMakeComplex(Fp_t re, Fp_t im)
{
	osk_fp_complex_t complex;
	complex.re = re;
	complex.im = im;
	return complex;
}

static inline osk_fp_complex_t fp_complex_add(const osk_fp_complex_t* a, const osk_fp_complex_t* b)
{
	osk_fp_complex_t x;
	x.re = a->re + b->re;
	x.im = a->im + b->im;
	return x;
}

static inline osk_fp_complex_t fp_complex_sub(const osk_fp_complex_t* a, const osk_fp_complex_t* b)
{
	osk_fp_complex_t x;
	x.re = a->re - b->re;
	x.im = a->im - b->im;
	return x;
}

static inline osk_fp_complex_t fp_complex_mult(const osk_fp_complex_t* a, const osk_fp_complex_t* b)
{
	osk_fp_complex_t x;
	//x.re = a->re * b->re - a->im * b->im;
	//x.re = FpMul(a->re, b->re) - FpMul(a->im, b->im);
	FpW_t re_w = (FpW_t)a->re * (FpW_t)b->re;
	re_w -= (FpW_t)a->im * (FpW_t)b->im;
	x.re = (Fp_t)(re_w >> FPSHFT);

	//x.im = a->re * b->im + a->im * b->re;
	//x.im = FpMul(a->re, b->im) + FpMul(a->im, b->re);
	FpW_t im_w = (FpW_t)a->re  * (FpW_t)b->im;
	im_w += (FpW_t)a->im * (FpW_t)b->re;
	x.im = (Fp_t)(im_w >> FPSHFT);

	return x;
}

static inline osk_fp_complex_t fp_complex_mult_fp(const osk_fp_complex_t* a, Fp_t b)
{
	osk_fp_complex_t x;
	x.re = FpMul(a->re, b) - FpMul(a->im, b);
	return x;
}

static inline osk_fp_complex_t fp_complex_div_fp(const osk_fp_complex_t* a, Fp_t b)
{
	osk_fp_complex_t x;
	x.re = FpDiv(a->re, b) - FpDiv(a->im, b);
	return x;
}

static inline osk_fp_complex_t fp_complex_r_shift(const osk_fp_complex_t* a, int n)
{
	osk_fp_complex_t x;
	x.re = FpRShift(a->re, n);
	x.im = FpRShift(a->im, n);
	return x;
}

static inline osk_fp_complex_t fp_complex_l_shift(const osk_fp_complex_t* a, int n)
{
	osk_fp_complex_t x;
	x.re = FpLShift(a->re, n);
	x.im = FpLShift(a->im, n);
	return x;
}

static inline void fp_complex_swap(osk_fp_complex_t* a, osk_fp_complex_t* b)
{
	FpSwap(&a->re, &b->re);
	FpSwap(&a->im, &b->im);
}

static inline char* fp_complex_str(const osk_fp_complex_t* a, char* buf, size_t buf_size)
{
	char buf_sub[32] = { '\0' };
	memset(buf, 0, buf_size);

	Fp2CStr(a->re, buf_sub, sizeof(buf_sub));
	StrNCat_S(buf, buf_size, buf_sub, strlen(buf_sub));
	const char* sep = ", ";
	StrNCat_S(buf, buf_size, sep, strlen(sep));

	Fp2CStr(a->im, buf_sub, sizeof(buf_sub));
	StrNCat_S(buf, buf_size, buf_sub, strlen(buf_sub));

	return buf;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
