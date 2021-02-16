/*
 * Type 'N Talk Text to Speech Engine
 * 
 * Scott Baker, http://www.smbaker.com/
 * 
 * This is an arduino implementation of the Votrax Type 'N Talk
 * standalone speech device. It is not an emulation of the SC-01A
 * itself, but relies on an actual real rare and expensive 
 * SC-01A to convert the phonemes to speech.
 */

#include <Arduino.h>
#include <Wire.h>
#include "defs.h"
#include "slave.h"
#include "display.h"
#include "amplifier.h"
#include "serial.h"
#include "speech.h"
#include "globals.h"

unsigned long tLoopTop;

void setup()
{
    DisplayInit();
    AmpInit();
    SpeechInit();
    SerialInit();
#ifdef SLAVE
    SlaveInit();
#endif

    SpeechAmpEnable(true);
    SpeechSystemReady();
}

void loop()
{
    tLoopTop = micros();

    SpeechUpdate();
    SerialUpdate();
}
