#include "OsakanaFftConfig.h"
#include <math.h>
#include <assert.h>
#include <algorithm>
#include "OsakanaFft.h"
#include "OsakanaFftUtil.h"
#include "bitreversetable.h"

// mbed has memset() in mbed.h
#ifdef __MBED__
#include <mbed.h>
// mbed doesn't have M_PI
#ifndef M_PI
#define M_PI           3.14159265358979323846f
#endif
#else
#include <stdlib.h>
#include <string.h>
#endif

struct _OsakanaFftContext_t {
	int N;
	int log2N;
	// twiddle factor table
	osk_complex_t* twiddles;
	osk_bitreverse_idx_pair_t* bitReverseIndexTable;
	uint16_t bitReverseIndexTableLen;
};

// W^n_N = exp(-i2pin/N)
// = cos(2 pi n/N) - isin(2 pi n/N)
static inline osk_complex_t twiddle(int n, int Nin)
{
	float theta = (float)(2.0f*M_PI*n / Nin);
	return MakeComplex((float)cos(theta), (float)-sin(theta));
}

void BitReverseTableForN(int N, int log2N, osk_bitreverse_idx_pair_t** ret_table, uint16_t* ret_count)
{
	uint16_t* table = (uint16_t*)malloc(sizeof(uint16_t) * N);
	memset(table, 0xFFFF, sizeof(uint16_t)*N);

	for (int i = 0; i < N; i++) {
		uint32_t r_idx = bitReverse(log2N, i);
		if (r_idx == i) {
			continue;
		}
		uint16_t idx_min = std::min(
			static_cast<uint16_t>(i),
			static_cast<uint16_t>(r_idx));
		uint16_t idx_max = std::max(
			static_cast<uint16_t>(i),
			static_cast<uint16_t>(r_idx));
		table[idx_min] = idx_max;
	}

	int item_num = 0;
	for (int i = 0; i < N; i++) {
		if (table[i] != 0xFFFF) {
			item_num++;
		}
	}

	*ret_table = (osk_bitreverse_idx_pair_t*)malloc(sizeof(osk_bitreverse_idx_pair_t) * (item_num));
	int counter = 0;
	for (uint16_t i = 0; i < N; i++) {
		if (table[i] == 0xFFFF) {
			continue;
		}
		(*ret_table)[counter].first = i;
		(*ret_table)[counter].second = table[i];
		counter++;
	}
	*ret_count = counter;

	free(table);
}

int InitOsakanaFft(OsakanaFftContext_t** pctx, int N, int log2N)
{
	int ret = 0;
	OsakanaFftContext_t* ctx = (OsakanaFftContext_t*)malloc(sizeof(OsakanaFftContext_t));
	if (ctx == NULL) {
		return -1;
	}

	memset(ctx, 0, sizeof(OsakanaFftContext_t));
	ctx->N = N;
	ctx->log2N = log2N;

	ctx->twiddles = (osk_complex_t*)malloc(sizeof(osk_complex_t) * N/2);
	if (ctx->twiddles == NULL) {
		ret = -2;
		goto exit_error;
	}

	for (int j = 0; j < N / 2; j++) {
		ctx->twiddles[j] = twiddle(j, N);
		//printf("twiddles[%d]=( %f, %f)\n", j, ctx->twiddles[j].re, ctx->twiddles[j].im);
	}

	BitReverseTableForN(N, log2N, &(ctx->bitReverseIndexTable), &(ctx->bitReverseIndexTableLen));

	*pctx = ctx;
	
	return 0;

exit_error:
	CleanOsakanaFft(ctx);
	ctx = NULL;
	*pctx = NULL;

	return ret;
}

void CleanOsakanaFft(OsakanaFftContext_t* ctx)
{
	if (ctx == NULL) {
		return;
	}

	free(ctx->twiddles);
	ctx->twiddles = NULL;
	free(ctx->bitReverseIndexTable);
	ctx->bitReverseIndexTable = NULL;

	free(ctx);
}

static inline void butterfly(osk_complex_t* r, const osk_complex_t* tf, int idx_a, int idx_b)
{
	osk_complex_t up = r[idx_a];
	osk_complex_t dn = r[idx_b];

	//r[idx_a] = up + tf * dn;
	//r[idx_b] = up - tf * dn;
	osk_complex_t dntf = complex_mult(&dn, tf);
	r[idx_a] = complex_add(&up, &dntf);
	r[idx_b] = complex_sub(&up, &dntf);
}

void OsakanaFft(const OsakanaFftContext_t* ctx, osk_complex_t* x)
{
	for (int i = 0; i < ctx->bitReverseIndexTableLen; i++) {
		const osk_bitreverse_idx_pair_t* pair = &ctx->bitReverseIndexTable[i];
		complex_swap(&x[pair->first], &x[pair->second]);
	}

	int dj = 2;
	int bnum = 1; // number of butterfly in 2nd loop
	int tw_idx_shift = ctx->log2N - 1;

	for (int i = 0; i < ctx->log2N; i++) {
		// j = 0,2,4,6
		// j = 0,4
		// j = 0
		for (int j = 0; j < ctx->N; j += dj) {
			int idx_a = j;
			int idx_b = j + bnum;
			for (int k = 0; k < bnum; k++) {

				//osk_complex_t tf = twiddle(k, dj);
				//butterfly(&f[0], &tf, idx_a++, idx_b++);

				int tw_idx = k << tw_idx_shift;
				osk_complex_t tf = ctx->twiddles[tw_idx];
				butterfly(&x[0], &tf, idx_a++, idx_b++);
			}
		}
		dj = dj << 1;
		bnum = bnum << 1;
		tw_idx_shift--;
	}
}

void OsakanaIfft(const OsakanaFftContext_t* ctx, osk_complex_t* x)
{
	for (int i = 0; i < ctx->bitReverseIndexTableLen; i++) {
		const osk_bitreverse_idx_pair_t* pair = &ctx->bitReverseIndexTable[i];
		complex_swap(&x[pair->first], &x[pair->second]);
	}

	int dj = 2;
	int bnum = 1;
	int tw_idx_shift = ctx->log2N - 1;

	for (int i = 0; i < ctx->log2N; i++) {
		for (int j = 0; j < ctx->N; j += dj) {
			int idx_a = j;
			int idx_b = j + bnum;
			for (int k = 0; k < bnum; k++) {
				
				/*osk_complex_t tf = twiddle(k, dj);
				tf.im = -tf.im;
				butterfly(&f[0], &tf, idx_a++, idx_b++);*/

				int tw_idx = k << tw_idx_shift;
				osk_complex_t tf = ctx->twiddles[tw_idx];
				tf.im = -tf.im;
				butterfly(&x[0], &tf, idx_a, idx_b);

				x[idx_a].re /= 2.0;
				x[idx_a].im /= 2.0;
				x[idx_b].re /= 2.0;
				x[idx_b].im /= 2.0;

				idx_a++;
				idx_b++;
			}
		}
		dj = dj << 1;
		bnum = bnum << 1;
		tw_idx_shift--;
	}
}
