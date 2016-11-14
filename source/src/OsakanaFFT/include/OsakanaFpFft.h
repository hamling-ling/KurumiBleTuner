#ifndef _OSAKANAFPFFT_H_
#define _OSAKANAFPFFT_H_

#include "OsakanaFpComplex.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _OsakanaFpFftContext_t OsakanaFpFftContext_t;

int InitOsakanaFpFft(OsakanaFpFftContext_t** pctx, int N, int log2N);
void CleanOsakanaFpFft(OsakanaFpFftContext_t* ctx);
void OsakanaFpFft(const OsakanaFpFftContext_t* ctx, osk_fp_complex_t* x, int scale);
void OsakanaFpIfft(const OsakanaFpFftContext_t* ctx, osk_fp_complex_t* x, int scale);
//void OsakanaFpFft(const OsakanaFpFftContext_t* ctx, osk_fp_complex_t* f, osk_fp_complex_t* F, int scale);
//void OsakanaFpIfft(const OsakanaFpFftContext_t* ctx, osk_fp_complex_t* F, osk_fp_complex_t* f, int scale);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
