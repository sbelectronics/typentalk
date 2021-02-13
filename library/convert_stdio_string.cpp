#include <string.h>

#include "convert_stdio_string.h"

StringStdioConverter::StringStdioConverter(void)
{
	*outputBuf = '\0';
}

void StringStdioConverter::emitPhoneme(uint8_t phoneme)
{
    if (*outputBuf) {
		char space = ' ';

		strncat(outputBuf, &space, 1);
	}
	strcat(outputBuf, phonemes[phoneme & 0x3F]);
}

