#ifndef __CONVERT_STDIO_H__
#define __CONVERT_STDIO_H__

#include "convert.h"

class StdioConverter: public Converter
{
 public:
  bool ModeEmit;

  StdioConverter(void) : Converter() { ModeEmit = true;}
  void SetEmit(bool b);
  virtual void emitPhoneme(uint8_t phoneme);
  virtual void putCharacter(char ch);
  virtual void DebugPrintf(const char *format, ...);
  virtual void DebugRule(const char *txt, uint8_t *myRulePtr, uint8_t *graPtr);
};

#endif
