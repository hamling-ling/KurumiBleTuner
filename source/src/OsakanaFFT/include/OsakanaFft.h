#ifndef _OSAKANAFFT_H_
#define _OSAKANAFFT_H_

#include "OsakanaComplex.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _OsakanaFftContext_t OsakanaFftContext_t;

int InitOsakanaFft(OsakanaFftContext_t** pctx, int N, int log2N);
void CleanOsakanaFft(OsakanaFftContext_t* ctx);
void OsakanaFft(const OsakanaFftContext_t* ctx, osk_complex_t* x);
void OsakanaIfft(const OsakanaFftContext_t* ctx, osk_complex_t* x);
//void OsakanaFft(const OsakanaFftContext_t* ctx, osk_complex_t* f, osk_complex_t* F);
//void OsakanaIfft(const OsakanaFftContext_t* ctx, osk_complex_t* F, osk_complex_t* f);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
