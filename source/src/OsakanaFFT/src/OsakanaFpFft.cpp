#include "OsakanaFftConfig.h"
#include <assert.h>
#include "OsakanaFpFft.h"
#include "OsakanaFftUtil.h"

#define USE_HARDCORD_TABLE

#if defined(USE_HARDCORD_TABLE)
#include "twiddletable.h"
#include "bitreversetable.h"
#else
#include "OsakanaFftUtil.h"
#endif

// mbed has memset() in mbed.h
#ifdef __MBED__
#include <mbed.h>
// mbed doesn't have M_PI
#ifndef M_PI
#define M_PI           3.14159265358979323846f
#endif
#endif

struct _OsakanaFpFftContext_t {
	int N;		// num of samples
	int log2N;	// log2(N)

#if defined(USE_HARDCORD_TABLE)
	// twiddle factor table
	const osk_fp_complex_t* twiddles;
	// bit reverse index table
	const osk_bitreverse_idx_pair_t* bitReverseIndexTable;
	uint16_t bitReverseIndexTableLen;
#else
	osk_fp_complex_t* twiddles;
	uint16_t* bitReverseIndexTable;
#endif

};

// W^n_N = exp(-i2pin/N)
// = cos(2 pi n/N) - isin(2 pi n/N)
static inline osk_fp_complex_t twiddle(int n, int Nin)
{
	float theta = (float)(2.0f*M_PI*n / Nin);
	osk_fp_complex_t ret;
	ret.re = Float2Fp((float)cos(theta));
	ret.im = Float2Fp((float)-sin(theta));

	return ret;
}

int InitOsakanaFpFft(OsakanaFpFftContext_t** pctx, int N, int log2N)
{
	int ret = 0;
	OsakanaFpFftContext_t* ctx = (OsakanaFpFftContext_t*)malloc(sizeof(OsakanaFpFftContext_t));
	if (ctx == NULL) {
		return -1;
	}

	memset(ctx, 0, sizeof(OsakanaFpFftContext_t));
	ctx->N = N;
	ctx->log2N = log2N;

#if defined(USE_HARDCORD_TABLE)
	ctx->twiddles = s_twiddlesFp[log2N-1];
	ctx->bitReverseIndexTable = s_bitReverseTable[log2N-1];
	ctx->bitReverseIndexTableLen = s_bitReversePairNums[log2N - 1];
#else
	ctx->twiddles = (osk_fp_complex_t*)malloc(sizeof(osk_fp_complex_t) * N/2);
	if (ctx->twiddles == NULL) {
		ret = -3;
		goto exit_error;
	}	printf("malloc items %d\n", sizeof(osk_fp_complex_t) * N / 2);// debug
	for (int j = 0; j < N/2; j++) {
		ctx->twiddles[j] = twiddle(j, N);
	}

	ctx->bitReverseIndexTable = (uint16_t*)malloc(sizeof(uint16_t) * N);
	if (ctx->bitReverseIndexTable == NULL) {
		ret = -4;
		goto exit_error;
	}printf("malloc reverse %d\n", sizeof(osk_fp_complex_t) * N);// debug
	for (int i = 0; i < N; i++) {
		ctx->bitReverseIndexTable[i] = (uint16_t)bitReverse(log2N, i);
	}
#endif
	*pctx = ctx;

	return 0;

#if !defined(USE_HARDCORD_TABLE)
exit_error:
	CleanOsakanaFpFft(ctx);
	ctx = NULL;
	*pctx = NULL;

	return ret;
#endif
}

void CleanOsakanaFpFft(OsakanaFpFftContext_t* ctx)
{
	if (ctx == NULL) {
		return;
	}

#if !defined(USE_HARDCORD_TABLE)
	free(ctx->twiddles);
	free(ctx->bitReverseIndexTable);
#endif
	ctx->twiddles = NULL;
	ctx->bitReverseIndexTable = NULL;

	free(ctx);
}

static inline void fp_butterfly(osk_fp_complex_t* r, const osk_fp_complex_t* tf, int idx_a, int idx_b)
{
	osk_fp_complex_t up = r[idx_a];
	osk_fp_complex_t dn = r[idx_b];
	osk_fp_complex_t dntf = fp_complex_mult(&dn, tf);

	r[idx_a] = fp_complex_add(&up, &dntf);
	r[idx_b] = fp_complex_sub(&up, &dntf);
}

void OsakanaFpFft(const OsakanaFpFftContext_t* ctx, osk_fp_complex_t* x, int scale)
{
	for (int i = 0; i < ctx->bitReverseIndexTableLen; i++) {
		const osk_bitreverse_idx_pair_t* pair = &ctx->bitReverseIndexTable[i];
		fp_complex_swap(&x[pair->first], &x[pair->second]);
	}

	int dj = 2;
	int bnum = 1;
	int tw_idx_shift = ctx->log2N - 1;

	for (int i = 0; i < ctx->log2N; i++) {
		for (int j = 0; j < ctx->N; j += dj) {
			int idx_a = j;
			int idx_b = j + bnum;
			for (int k = 0; k < bnum; k++) {

				int tw_idx = k << tw_idx_shift;
				osk_fp_complex_t tf = ctx->twiddles[tw_idx];

				fp_butterfly(&x[0], &tf, idx_a, idx_b);

				x[idx_a] = fp_complex_r_shift(&x[idx_a], scale);
				x[idx_b] = fp_complex_r_shift(&x[idx_b], scale);

				idx_a++;
				idx_b++;
			}
		}
		dj = dj << 1;
		bnum = bnum << 1;
		tw_idx_shift--;
	}
}


void OsakanaFpIfft(const OsakanaFpFftContext_t* ctx, osk_fp_complex_t* x, int scale)
{
	for (int i = 0; i < ctx->bitReverseIndexTableLen; i++) {
		const osk_bitreverse_idx_pair_t* pair = &ctx->bitReverseIndexTable[i];
		fp_complex_swap(&x[pair->first], &x[pair->second]);
	}

	int dj = 2;
	int bnum = 1;
	int tw_idx_shift = ctx->log2N - 1;

	for (int i = 0; i < ctx->log2N; i++) {
		for (int j = 0; j < ctx->N; j += dj) {
			int idx_a = j;
			int idx_b = j + bnum;
			for (int k = 0; k < bnum; k++) {

				int tw_idx = k << tw_idx_shift;
				osk_fp_complex_t tf = ctx->twiddles[tw_idx];
				tf.im = -tf.im;

				fp_butterfly(&x[0], &tf, idx_a, idx_b);

				// div f[idx_a] by 2 instead of div by N end of func
				x[idx_a] = fp_complex_l_shift(&x[idx_a], 1 - scale);
				x[idx_b] = fp_complex_l_shift(&x[idx_b], 1 - scale);

				idx_a++;
				idx_b++;
			}
		}
		dj = dj << 1;
		bnum = bnum << 1;
		tw_idx_shift--;
	}

}
#if 0

void OsakanaFpFft(const OsakanaFpFftContext_t* ctx, osk_fp_complex_t* f, osk_fp_complex_t* F, int scale)
{
	memcpy(F, f, sizeof(osk_fp_complex_t) * ctx->N);

	for (int i = 0; i < ctx->bitReverseIndexTableLen; i++) {
		const osk_bitreverse_idx_pair_t* pair = &ctx->bitReverseIndexTable[i];
		F[pair->first] = f[pair->second];
		F[pair->second] = f[pair->first];
	}

	int dj = 2;
	int bnum = 1;
	int tw_idx_shift = ctx->log2N - 1;

	for (int i = 0; i < ctx->log2N; i++) {
		for (int j = 0; j < ctx->N; j += dj) {
			int idx_a = j;
			int idx_b = j + bnum;
			for (int k = 0; k < bnum; k++) {

				int tw_idx = k << tw_idx_shift;
				osk_fp_complex_t tf = ctx->twiddles[tw_idx];

				fp_butterfly(&F[0], &tf, idx_a, idx_b);

				F[idx_a] = fp_complex_r_shift(&F[idx_a], scale);
				F[idx_b] = fp_complex_r_shift(&F[idx_b], scale);

				idx_a++;
				idx_b++;
			}
		}
		dj = dj << 1;
		bnum = bnum << 1;
		tw_idx_shift--;
	}
}


void OsakanaFpIfft(const OsakanaFpFftContext_t* ctx, osk_fp_complex_t* F, osk_fp_complex_t* f, int scale)
{
	memcpy(f, F, sizeof(osk_fp_complex_t) * ctx->N);

	for (int i = 0; i < ctx->bitReverseIndexTableLen; i++) {
		const osk_bitreverse_idx_pair_t* pair = &ctx->bitReverseIndexTable[i];
		f[pair->first] = F[pair->second];
		f[pair->second] = F[pair->first];
	}

	int dj = 2;
	int bnum = 1;
	int tw_idx_shift = ctx->log2N - 1;

	for (int i = 0; i < ctx->log2N; i++) {

		for (int j = 0; j < ctx->N; j += dj) {
			int idx_a = j;
			int idx_b = j + bnum;
			for (int k = 0; k < bnum; k++) {

				int tw_idx = k << tw_idx_shift;
				osk_fp_complex_t tf = ctx->twiddles[tw_idx];
				tf.im = -tf.im;

				fp_butterfly(&f[0], &tf, idx_a, idx_b);

				// div f[idx_a] by 2 instead of div by N end of func
				f[idx_a] = fp_complex_l_shift(&f[idx_a], 1 - scale);
				f[idx_b] = fp_complex_l_shift(&f[idx_b], 1 - scale);

				idx_a++;
				idx_b++;
			}
		}
		dj = dj << 1;
		bnum = bnum << 1;
		tw_idx_shift--;
	}
}
#endif

