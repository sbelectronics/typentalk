#include <string.h>
#include "phoneme.h"

#include "convert_stdio_string.h"

StringStdioConverter::StringStdioConverter(void)
{
	*outputBuf = '\0';
}

void StringStdioConverter::emitPhoneme(uint8_t phoneme)
{
	char phonemeBuf[MAX_PHONEME_BUF];
  if (*outputBuf) {
		char space = ' ';

		strncat(outputBuf, &space, 1);
	}
	if (getPhoneme(phoneme & 0x3F, phonemeBuf)) {
	    strcat(outputBuf, phonemeBuf);
	}
}

