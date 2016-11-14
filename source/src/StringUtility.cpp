#include "StringUtility.h"
#include <Arduino.h>
#include <cassert>

using namespace std;

const char* getCommaSeparatedWord(const char* str, int *wordLen)
{
	assert(str != NULL);

	int len = strlen(str);
	assert(1 <= len);

	*wordLen = 0;
	int idx_first = 0;
	int idx_last = len - 1;
	for (int i = 1; i < len; i++) {
		if (str[i] == ',') {
			idx_last = i-1;
			break;
		}
	}
	*wordLen = idx_last - idx_first + 1;
	return &(str[idx_first]);
}

char* copyCommaSeparatedWord(const char* str, char* buf, int buf_len)
{
	assert(str != NULL);
	assert(buf != NULL);

	int len = 0;
	const char* ptr = getCommaSeparatedWord(str, &len);

	strncpy(buf, ptr, min(buf_len,len));
	if (len >= buf_len) {
		buf[buf_len - 1] = '\0';
	}
	else {
		buf[len] = '\0';
	}
	return buf;
}

void itoaUi16(uint16_t val, char* output)
{
    char buf[16] = {0};
    itoa((int)val, buf, 16);

    if(strlen(buf) == 1) {
        output[0] = '0';
        output[1] = buf[0];
    } else {
        output[0] = buf[0];
        output[1] = buf[1];
    }
}

void itoaUi8(uint8_t val, char* output)
{
    char buf[16] = {0};
    itoa((int)val, buf, 16);
    output[0] = buf[0];
}
