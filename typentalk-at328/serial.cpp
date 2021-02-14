#include <Arduino.h>
#include <Wire.h>
#include <util/crc16.h>
#include "speech.h"
#include "serial.h"
#include "../library/convert.h"
#include "../library/typetalk.h"
//#include "amplifier.h"
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
};


#define INPUTBUF_SIZE 750
char inputBuffer[INPUTBUF_SIZE];

uint8_t LastSerialByte;

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

// static declare these so they get accounted for in RAM usage
ArduinoConverter GloConverter;
ArduinoTypeTalk GloTypeTalk (&GloConverter);

void SerialUpdate()
{
  if (Serial.available() > 0) {
    uint8_t data = Serial.read();

    GloTypeTalk.bufferCharacter(data);

    LastSerialByte = data;
  }
}

void SerialInit() {

    GloTypeTalk.initBuffer(inputBuffer, INPUTBUF_SIZE);

    LastSerialByte = 0xFF;
    Serial.begin(9600);

    Serial.write("test\r\n");
}