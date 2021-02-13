#include <Arduino.h>
#include <Wire.h>
#include <util/crc16.h>
#include "speech.h"
#include "serial.h"
#include "../library/convert.h"
#include "../library/typetalk.h"
//#include "amplifier.h"
//#include "slave.h"

#define INPUTBUF_SIZE 750
char inputBuffer[INPUTBUF_SIZE];

uint8_t LastSerialByte;
Converter *GloConverter = NULL;
TypeTalk *GloTypeTalk = NULL;

void SerialUpdate()
{
  if (Serial.available() > 0) {
    uint8_t data = Serial.read();

    GloTypeTalk->handleCharacter(data);

    LastSerialByte = data;
  }
}

void SerialInit() {
    GloConverter = new Converter();
    GloTypeTalk = new TypeTalk(GloConverter);

    GloTypeTalk->initBuffer(inputBuffer, INPUTBUF_SIZE);

    LastSerialByte = 0xFF;
    Serial.begin(9600);
}