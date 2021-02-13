#ifndef __TYPETALK_H__
#define __TYPETALK_H__

#include "tt_types.h"
#include "convert.h"

#define MAX_WORDBUF 32

class TypeTalk {
	public:
	    TypeTalk(Converter *aConverter);

        void handleCharacter(char ch);

	protected:
        Converter *converter;
        uint8_t State;
        bool ModePSend;
        bool ModeEcho;
        bool ModeCaps;
        char wordBuf[MAX_WORDBUF+1];

        void handleEscape(char ch);
        void init();
};

#endif
