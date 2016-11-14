#ifndef _DEBUG_H_
#define _DEBUG_H_

#if 0
#define LOG(...)     Serial.println(__VA_ARGS__)
#define DEBUG(...) { char debug_buf[256] = {0}; snprintf(debug_buf, sizeof(debug_buf), __VA_ARGS__); LOG(debug_buf); }
#else
#define LOG(...)
#define DEBUG(...)
#endif

#endif	// _DEBUG_H_
