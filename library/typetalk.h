#ifndef __TYPETALK_H__
#define __TYPETALK_H__

#include "tt_types.h"
#include "convert.h"

#define MAX_WORDBUF 32

class TypeTalk {
    public:
        bool ModePSend;
        bool ModeEcho;
        bool ModeCaps;

	TypeTalk(Converter *aConverter);

        void handleCharacter(char ch);
        virtual void putCharacter(char ch);

        void setPSend(bool b) { ModePSend = b; }
        void setEcho(bool b) { ModeEcho = b; }
        void setCaps(bool b) { ModeCaps = b; }

    protected:
        Converter *converter;
        uint8_t State;
        char wordBuf[MAX_WORDBUF+1];

        void handleEscape(char ch);
        void init();
};

#endif
