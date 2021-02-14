#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

#include "defs.h"
#include "display.h"
#include "../library/phoneme.h"
#include "globals.h"

/*
 * If Alpha4 is defined, then we will act as an I2C master
 * and support output to the Adafruit Alphanumeric (14-segment)
 * backpack.
 */

#ifdef ALPHA4

Adafruit_AlphaNum4 alpha4;

void DisplayInit()
{
    alpha4 = Adafruit_AlphaNum4();
    alpha4.begin(0x70);
}

void DisplayUpdate(uint8_t phoneme)
{
    char phonemeBuf[MAX_PHONEME_BUF];

    if (getPhoneme(phoneme, phonemeBuf)) {
        DisplayWriteString(phonemeBuf);
    }
}

void DisplayWriteString(char *word)
{
    uint8_t len;
    uint8_t i;
    len = strlen(word);

    alpha4.clear();

    for (i=0; i<4; i++) {
        if (len>i) {
            alpha4.writeDigitAscii(i, toupper(word[i]));
        } else {
        }
    }
    alpha4.writeDisplay();
}

void DisplayClear()
{
    alpha4.clear();
    alpha4.writeDisplay(); 
}

#else

void DisplayInit()
{
}

void DisplayUpdate(uint8_t, uint8_t)
{
}

void DisplayWriteString(char *)
{
}

void DisplayClear()
{
}

#endif