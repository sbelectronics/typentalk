#ifndef __SPEECH_H
#define __SPEECH_H

extern const uint8_t DAISY[255] PROGMEM;

extern uint8_t SpeechLastPhonemeOut;
extern uint8_t SpeechBufCount;
extern uint8_t SpeechState;
extern bool SpeechWritePA0OnEmpty;

void SpeechUpdate();
void SpeechInit();
void SpeechAmpEnable(bool);
void SpeechProgMem(const uint8_t *p, uint8_t count);
void SpeechSystemReady();
bool SpeechBufInsert(uint8_t data);

#endif
