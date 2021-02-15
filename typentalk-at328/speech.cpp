#include <Arduino.h>
#include <Wire.h>
#include <util/crc16.h>
#include "speech.h"
#include "display.h"
#include "amplifier.h"

// AR goes from HIGH to LOW following strobe
// AR hoes from LOW back to HIGH when phoneme is complete
#define AR_PIN (1<<PC0)

#define PHONEME_MASK ((1<<PD2) | (1<<PD3) | (1<<PD4) | (1<<PD5) | (1<<PD6) | (1<<PD7))
#define PHONEME_SHIFT 2

#define INFLECTION_MASK ((1<<PB0) | (1<<PB1))

// LOW-to-HIGH transition on strobe will pull in the phoneme
#define STROBE_PIN (1<<PB2)

// HIGH on SHDN enables the amplifier
#define SHDN_PIN (1<<PC1)

#define SPEECHBUF_SIZE 255

#define SPEECH_INIT             0x00
#define SPEECH_BUSY             0x01
#define SPEECH_IDLE             0x02
#define SPEECH_BUSYWAIT         0x03
#define SPEECH_ACKED            0x04

#define SPEECH_ERR_MASK         0x10  // high bit indicates halted due to error
#define SPEECH_ERR_BUSY_TIMEOUT 0x15

uint8_t SpeechBuf[SPEECHBUF_SIZE];
uint8_t *SpeechBufHead;
uint8_t *SpeechBufTail;
uint8_t *SpeechBufEnd = &SpeechBuf[SPEECHBUF_SIZE-1];  // the last valid position in the buffer
uint8_t SpeechBufCount;
uint8_t SpeechState;
bool SpeechWritePA0OnEmpty;

uint8_t SpeechLastPhonemeOut;

const uint8_t THIS_IS_A_TEST[18] PROGMEM = {0x38, 0x09, 0x0A, 0x1F,          // This
                                0x3E,
                                0x27, 0x12,                          // Is
                                0x3E,
                                0x06, 0x21, 0x29,                    // A
                                0x3E,
                                0x2A, 0x02, 0x00, 0x1F, 0x2A,        // Test
                                0x3F};                               // Stop

const uint8_t SYSTEM_READY[13] PROGMEM = {0x3E, 0x1F, 0x0A, 0x1F, 0x2A, 0x02, 0x0C, 0x03, 0x2B, 0x02, 0x1E, 0x29, 0x03};

const uint8_t DAISY[255] PROGMEM = {
        0xDE, 0xC6, 0xE1, 0xE1, 0xE1, 0xE1, 0xE1, 0xE1, 
        0xD2, 0xD2, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0x03, 
        0x9E, 0x86, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 
        0x52, 0x52, 0x29, 0x29, 0x29, 0x29, 0x29, 0x6A, 
        0x42, 0x40, 0x58, 0x4C, 0xBC, 0xA9, 0xA9, 0xB4, 
        0xB4, 0xBA, 0x3F, 0x6F, 0x7B, 0x7B, 0x8D, 0x9F, 
        0xBA, 0xAB, 0x2A, 0x2B, 0x36, 0x28, 0x28, 0x28, 
        0x3E, 0x3E, 0x3E, 0x03, 0x55, 0x40, 0x49, 0x62, 
        0x62, 0x62, 0x62, 0x62, 0x4C, 0xDB, 0xEF, 0xFB, 
        0xC2, 0xC1, 0xDD, 0xD9, 0xEB, 0xC6, 0xE1, 0xE9, 
        0xE9, 0xE9, 0xE9, 0x92, 0xA9, 0xA9, 0xA9, 0xA9, 
        0xA9, 0x43, 0x43, 0x7D, 0x58, 0x7F, 0x9D, 0xB5, 
        0xAB, 0x3F, 0xB8, 0xB2, 0xB2, 0x3F, 0xD8, 0xF3, 
        0xF2, 0xF2, 0xCF, 0x72, 0x72, 0x4F, 0x7F, 0x22, 
        0x36, 0x28, 0x28, 0x37, 0x3E, 0x3E, 0x3E, 0x3E, 
        0x0B, 0x09, 0x2A, 0xAD, 0xB5, 0x8D, 0xAA, 0x4E, 
        0x7C, 0x69, 0x7F, 0x33, 0x3F, 0xDF, 0xEA, 0xD5, 
        0xA9, 0xA9, 0xA2, 0xA2, 0xD8, 0xA7, 0x91, 0x4C, 
        0x41, 0x41, 0x6B, 0x27, 0x1E, 0x1A, 0x3E, 0x3E, 
        0x3E, 0x3E, 0x03, 0x15, 0x00, 0x09, 0x29, 0x29, 
        0x59, 0xAF, 0xBB, 0x8D, 0xEA, 0x03, 0xB3, 0x5D, 
        0x66, 0x7A, 0x6B, 0x5E, 0x3E, 0x33, 0x03, 0x99, 
        0x81, 0x40, 0xAB, 0x27, 0x1E, 0x1A, 0x3E, 0x3E, 
        0x3E, 0x3E, 0x0E, 0x32, 0x71, 0x6A, 0xA2, 0xB6, 
        0xB7, 0xB7, 0x98, 0x3F, 0x98, 0x96, 0x96, 0x99, 
        0x5F, 0x6D, 0x7C, 0x69, 0x6A, 0x3E, 0x03, 0x32, 
        0x25, 0x55, 0x72, 0x4D, 0x43, 0xF8, 0xF2, 0xF1, 
        0x9F, 0xBC, 0xA1, 0xAA, 0x3E, 0xB2, 0xA3, 0x8F, 
        0x86, 0xA1, 0xA9, 0xCE, 0xE3, 0xC8, 0xE9, 0xE2, 
        0xDF, 0x8B, 0x99, 0x63, 0x58, 0x8E, 0x8B, 0x8A, 
        0x98, 0xAA, 0x1D, 0x34, 0x34, 0x2B, 0xAA, 0xB6, 
        0xA8, 0xA8, 0xB7, 0xB7, 0x3E, 0x3E, 0x3F
        };
                 

bool SpeechBufIsFull()
{
    bool retVal;

    cli();
    retVal = (SpeechBufCount == SPEECHBUF_SIZE);

    sei();
    return retVal;
}

bool SpeechBufIsEmpty()
{
    bool retVal;

    cli();
    retVal = (SpeechBufCount == 0);

    sei();
    return retVal;
}

bool SpeechBufInsert(uint8_t data)
{
    bool retVal;
    cli();

    if (SpeechBufCount == SPEECHBUF_SIZE) {
        retVal = false;
    } else {
        *SpeechBufHead = data;

        if (SpeechBufHead == SpeechBufEnd) {
            SpeechBufHead = SpeechBuf;
        } else {
            SpeechBufHead++;
        }

        SpeechBufCount++;
        retVal = true;
    }

    sei();
    return retVal;
}

uint8_t SpeechBufRemove()
{
    uint8_t data;

    cli();

    if (SpeechBufCount==0) {
        data = 0;
    } else {
        data = *SpeechBufTail;
        if (SpeechBufTail == SpeechBufEnd) {
            SpeechBufTail = SpeechBuf;
        } else {
            SpeechBufTail++;
        }

        SpeechBufCount--;
    }

    sei();
    return data;
}

void SpeechAmpEnable(bool enable) 
{
    if (enable) {
        // check to see if it's already enabled
        if ((PORTC & SHDN_PIN) == 0) {
            PORTC |= SHDN_PIN;
            delay(1); // give it a millisecond before sending I2C commands
            AmpSetup();
        }        
    } else {
        PORTC &= (~SHDN_PIN);
    }
}

void StrobeLow()
{
    PORTB &= (~STROBE_PIN);
}

void StrobeHigh()
{
    PORTB |= STROBE_PIN;
}

void OutputPhoneme(uint8_t phoneme)
{
    phoneme = phoneme & 0x3F;
    PORTD = (PORTD & ~PHONEME_MASK) | (phoneme<<PHONEME_SHIFT);
}

void OutputInflection(uint8_t inflection)
{
    // invert inflection due to transistor drivers
    inflection = (~inflection) & 0x03;
    PORTB = (PORTB & ~INFLECTION_MASK) | inflection;
}

void WritePhoneme(uint8_t data)
{
    uint8_t busyWaitCounter;

    SpeechLastPhonemeOut = data;
    OutputPhoneme(data & 0x3F);
    OutputInflection(data >> 6);
    StrobeHigh();

    busyWaitCounter = 255;
    while (((PINC & AR_PIN) != 0) && (busyWaitCounter>0)) {
        SpeechState = SPEECH_BUSYWAIT;
        busyWaitCounter--;
        delayMicroseconds(1);
    }
    
    if (busyWaitCounter == 0) {
        SpeechState = SPEECH_ERR_BUSY_TIMEOUT;
    } else {
        SpeechState = SPEECH_ACKED;
    }

    StrobeLow();
    OutputPhoneme(0x00); // reset phonemes - remember that odd high power consumption??
}

void SpeechUpdate()
{
    if ((SpeechState & SPEECH_ERR_MASK) == SPEECH_ERR_MASK) {
        // error state, do not update
        return;
    }

    if ((PINC & AR_PIN) == 0) {
        SpeechState = SPEECH_BUSY;
        return;
    }

    if (SpeechBufIsEmpty()) {
        if (SpeechState != SPEECH_IDLE) {
            // Write a PA0 on empty buffer
            // This is what the type'n talk does...
            if (SpeechWritePA0OnEmpty) {
                WritePhoneme(0x03);
            }
            DisplayClear();
            SpeechState = SPEECH_IDLE;
        }
        return;
    }

    // votrax is ready for data, and there are phonemes to speak
    uint8_t data = SpeechBufRemove();

    WritePhoneme(data);
    DisplayUpdate(data & 0x3F);
}

void SpeechInit()
{
    SpeechBufHead = SpeechBuf;
    SpeechBufTail = SpeechBuf;
    SpeechBufCount = 0;
    SpeechLastPhonemeOut = 0;
    SpeechWritePA0OnEmpty = false;

    // shdn is output, default is low (amp is shutdown)
    PORTC &= (~SHDN_PIN);
    DDRC |= SHDN_PIN;

    StrobeLow();
    DDRB |= STROBE_PIN;

    OutputPhoneme(0x00);
    DDRD |= PHONEME_MASK;

    OutputInflection(0x00);
    DDRB |= INFLECTION_MASK;

    SpeechState = SPEECH_INIT;
}

void SpeechProgMem(const uint8_t *p, uint8_t count)
{
    int i;

    for (i=0; i<count; i++) {
      uint8_t phoneme = pgm_read_byte_near(p);
      p++;
      SpeechBufInsert(phoneme);
    }
}

void SpeechSystemReady()
{
    SpeechProgMem(SYSTEM_READY, 13) ;
}
