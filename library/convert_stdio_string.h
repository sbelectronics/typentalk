#ifndef __convert_string_h__
#define __convert_string_h__

#include "convert_stdio.h"

class StringStdioConverter: public StdioConverter
{
	public:
        char outputBuf[MAX_GRAPHENEBUF*4];

	    StringStdioConverter(void);

		virtual void emitPhoneme(uint8_t phoneme);
};

#endif
