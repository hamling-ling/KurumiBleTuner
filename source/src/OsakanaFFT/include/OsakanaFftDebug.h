#ifndef _OSAKANAFFTDEBUG_H_
#define _OSAKANAFFTDEBUG_H_

#define ILOG(...)			{ \
								char debug_output_buf2_[128] = { 0 }; \
								snprintf(debug_output_buf2_,sizeof(debug_output_buf2_), __VA_ARGS__); \
								LOG_PRINTF(debug_output_buf2_); \
								LOG_PRINTF(LOG_NEWLINE); \
							}

#if defined(_DEBUG)

#if !defined(LOG_NEWLINE) 
#define LOG_NEWLINE "\n"
#endif

#if defined(LOG_PRINTF)

#define DLOG(...)			{ \
								char debug_output_buf2_[128] = { 0 }; \
								snprintf(debug_output_buf2_,sizeof(debug_output_buf2_), __VA_ARGS__); \
								LOG_PRINTF(debug_output_buf2_); \
								LOG_PRINTF(LOG_NEWLINE); \
							}
#else
#define DLOG(fmt, ...)		fprintf(stderr, fmt LOG_NEWLINE, __VA_ARGS__);
#endif

#define DCOMPLEX(cs, num) 	{ \
								char debug_output_buf_[128] = { 0 }; \
								for (int i = 0; i < num; i++) { \
									complex_str(&cs[i], debug_output_buf_, sizeof(debug_output_buf_)); \
									DLOG("%s", debug_output_buf_); \
								} \
							}
#define DFPS(x, num)		{ \
								for (int i = 0; i < num; i++) { \
									DLOG("%f", x[i]); \
								} \
							}
#define DRAWDATA(x, num)	{ \
								for (int i = 0; i < num; i++) {	\
 									DLOG("%u", x[i].re); \
 								} \
							}
#define DRAWDATAf(x, num)	{ \
								for (int i = 0; i < num; i++) {	\
 									DLOG("%f", x[i].re); \
 								} \
							}
#else

#define DLOG(fmt, ...)
#define DCOMPLEX(cs, num)
#define DFPS(fps, num)
#define DRAWDATA(x, num)
#define DRAWDATAf(x, num)
#endif

#endif

