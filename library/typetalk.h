#ifndef __TYPETALK_H__
#define __TYPETALK_H__

#include "tt_types.h"
#include "ext_escape.h"
#include "convert.h"

#define MAX_WORDBUF 32

class TypeTalk {
    public:
        bool ModePSend;
        bool ModeEcho;
        bool ModeCaps;
        bool ModeTimer;
        bool ModeCRLF;

        bool inEscapeSequence;

        char *inputBuffer;
        char *inputBufPtr;
        int inputBufLen;
        int inputBufSize;
        char lastInputChar;

	TypeTalk(Converter *aConverter);

        void initBuffer(char *bufPtr, int bufLen);

        void bufferCharacter(char ch);
        void handleCharacter(char ch);
        void echoCharacter(char ch);
        virtual void putCharacter(char ch);

        virtual void executeEscape(uint8_t escapeOp);

        void processLine();

        bool hasData() { return inputBufLen>0; }

        void setPSend(bool b) { ModePSend = b; }
        void setEcho(bool b) { ModeEcho = b; }
        void setCaps(bool b) { ModeCaps = b; }

    protected:
        Converter *converter;
        uint8_t State;
        char *wordBuf;

        void handleEscapeCharacter(char ch);
        void init();
};

#endif
