#ifndef __TYPETALK_STDIO_H__
#define __TYPETALK_STDIO_H__

#include "typetalk.h"

class StdioTypeTalk: public TypeTalk {
  public:
  	StdioTypeTalk(Converter *aConverter) : TypeTalk(aConverter) {}
    virtual void putCharacter(char ch);
};

#endif
