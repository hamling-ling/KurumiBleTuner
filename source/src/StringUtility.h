#ifndef _STRINGUTILITY_H_
#define _STRINGUTILITY_H_

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

const char* getCommaSeparatedWord(const char* str, int *wordLen);
char* copyCommaSeparatedWord(const char* str, char* buf, int buf_len);
void itoaUi16(uint16_t val, char* output);
void itoaUi8(uint8_t val, char* output);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
