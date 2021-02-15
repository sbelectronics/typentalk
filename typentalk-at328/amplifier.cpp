#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TPA2016.h>

#include "defs.h"
#include "amplifier.h"
#include "display.h"
#include "globals.h"

/*
 * If Alpha4 is defined, then we will act as an I2C master
 * and support output to the Adafruit Alphanumeric (14-segment)
 * backpack.
 */

#ifdef AMPLIFIER

Adafruit_TPA2016 audioamp;

uint8_t AmplifierVolume;
bool AmplifierSpeakerEnabled;

void AmpInit()
{
    audioamp = Adafruit_TPA2016();
    audioamp.begin();

    AmplifierVolume = AMP_GAIN_ZERO;
    AmplifierSpeakerEnabled = true; // default state is speakers enabled
}

void AmpSetup()
{
    // Note: These do _NOTHING_ if the shutdown pin is
    // low while the commands are sent.

    // Note: I think the minimum release time might be 1,
    // which according to the datashseet is about 0.738 seconds
    // to ramp 90%.
    
    audioamp.setAGCCompression(TPA2016_AGC_OFF);
    audioamp.setReleaseControl(0);
    audioamp.setAttackControl(0);
    audioamp.setHoldControl(0);
    audioamp.setLimitLevelOff();
    audioamp.setNoiseGateThreshold(0);
    audioamp.enableNoiseGate(false);    

    AmpSetVolume(AmplifierVolume, false);
}

void AmpSetVolume(uint8_t volume, bool updateDisplay)
{
    char tmp[16];
    int iVolume = volume;

    // range for the amplifier is -28 to +30
    iVolume -= 28;
    if (iVolume < -28) {
        iVolume = -28;
    }
    if (iVolume > 30) {
        iVolume = 30;
    }

    audioamp.setGain(iVolume);

    if (updateDisplay) {
        sprintf(tmp, "G%d", iVolume);
        DisplayWriteString(tmp);
    }
}

void AmpEnableSpeaker(bool enable)
{
    if (enable == AmplifierSpeakerEnabled) {
        // already set
        return;
    }
    audioamp.enableChannel(enable, enable);
    AmplifierSpeakerEnabled = enable;
}

#else

void AmpInit()
{
}

void AmpSetup()
{
}

void AmpSetVolume(uint8_t)
{
}

void AmpEnableSpeaker(bool)
{
}

#endif