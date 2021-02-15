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
