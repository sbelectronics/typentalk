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

        char *inputBuffer;
        char *inputBufPtr;
        int inputBufLen;
        int inputBufSize;
        char lastInputChar;

	TypeTalk(Converter *aConverter);

        void initBuffer(char *bufPtr, int bufLen);

        void bufferCharacter(char ch);
        void handleCharacter(char ch);
        virtual void putCharacter(char ch);

        void setPSend(bool b) { ModePSend = b; }
        void setEcho(bool b) { ModeEcho = b; }
        void setCaps(bool b) { ModeCaps = b; }

    protected:
        Converter *converter;
        uint8_t State;
        char *wordBuf;

        void processLine();
        void handleEscape(char ch);
        void init();
};

#endif
