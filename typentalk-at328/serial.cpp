#include <Arduino.h>
#include <Wire.h>
#include <util/crc16.h>
#include "globals.h"
#include "speech.h"
#include "serial.h"
#include "../library/convert.h"
#include "../library/typetalk.h"
#include "../library/ext_escape.h"
#include "display.h"
#include "amplifier.h"
//#include "slave.h"

class ArduinoConverter: public Converter {
  public:
  		virtual void emitPhoneme(uint8_t phoneme);
		  virtual void putCharacter(char ch);
};

class ArduinoTypeTalk: public TypeTalk {
  public:
      ArduinoTypeTalk(Converter *aConverter) : TypeTalk(aConverter) {}
		  virtual void putCharacter(char ch);
      virtual void executeEscape(uint8_t escapeOp);
};


#define INPUTBUF_SIZE 750
char inputBuffer[INPUTBUF_SIZE];

uint8_t LastSerialByte;
unsigned long LastSerialUpdate;
unsigned long SerialIdle;

void ArduinoConverter::emitPhoneme(uint8_t phoneme)
{
  SpeechWritePA0OnEmpty = true; // force this in type'n talk mode
  SpeechBufInsert(phoneme);
}

void ArduinoConverter::putCharacter(char ch)
{
  Serial.write(ch);
}

void ArduinoTypeTalk::putCharacter(char ch)
{
  Serial.write(ch);
}

void ArduinoTypeTalk::executeEscape(uint8_t escapeOp)
{
    switch (escapeOp) {
      case ESC_VOL:
        AmpSetVolume(escapeParam, true);
        break;
      case ESC_DAISY:
        SpeechProgMem(DAISY, 255);
        break;
      case ESC_RESET:
        TypeTalk::executeEscape(escapeOp);
        SpeechSystemReady();
        break;
      default:
        TypeTalk::executeEscape(escapeOp);
        break;
    }
}

// static declare these so they get accounted for in RAM usage
ArduinoConverter GloConverter;
ArduinoTypeTalk GloTypeTalk (&GloConverter);

void SerialUpdate()
{
  if (Serial.available() > 0) {
    uint8_t data = Serial.read();

    GloTypeTalk.bufferCharacter(data);

    LastSerialByte = data;

    SerialIdle = 0;
  } else {
    // no data. Check for idle timeout.
    if (GloTypeTalk.ModeTimer && GloTypeTalk.hasData() && (tLoopTop > LastSerialUpdate)) {
        unsigned long elapsed = tLoopTop - LastSerialUpdate;
        SerialIdle = SerialIdle + elapsed;
        if (SerialIdle > 4000000) {
          GloTypeTalk.processLine();
        }
    }
  }

  LastSerialUpdate = tLoopTop;
}

void SerialInit() {

    GloTypeTalk.initBuffer(inputBuffer, INPUTBUF_SIZE);

    LastSerialUpdate = 0;
    SerialIdle = 0;

    LastSerialByte = 0xFF;
    Serial.begin(9600);

    Serial.write("test\r\n");
}