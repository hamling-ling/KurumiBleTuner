#ifndef _OSAKANAFPFFTDEBUG_H_
#define _OSAKANAFPFFTDEBUG_H_

#include "OsakanaFftDebug.h"

#if defined(_DEBUG)
#define DCOMPLEXFp(cs, num) { \
								char debug_output_buf_[128] = { 0 }; \
								for (int i = 0; i < num; i++) { \
									fp_complex_str(&cs[i], debug_output_buf_, sizeof(debug_output_buf_)); \
									DLOG("%s", debug_output_buf_); \
								} \
							}
#define DFPSFp(fps, num)	{ \
								char debug_output_buf_[128] = { 0 }; \
								for (int i = 0; i < num; i++) { \
									Fp2CStr(fps[i], debug_output_buf_, sizeof(debug_output_buf_)); \
									DLOG("%s", debug_output_buf_); \
								} \
							}
#else
#define DCOMPLEXFp(cs, num)
#define DFPSFp(fps, num)
#endif

#endif
