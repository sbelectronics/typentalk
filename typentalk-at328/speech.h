#ifndef __SPEECH_H
#define __SPEECH_H

extern uint8_t SpeechLastPhonemeOut;
extern uint8_t SpeechBufCount;
extern uint8_t SpeechState;
extern bool SpeechWritePA0OnEmpty;

void SpeechUpdate();
void SpeechInit();
void SpeechAmpEnable(bool);
void SpeechTest();
bool SpeechBufInsert(uint8_t data);

#endif
