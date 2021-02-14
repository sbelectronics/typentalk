#include <string.h>
#include "phoneme.h"

#ifdef ARDUINO
#define PHONEMEMEM PROGMEM
#else
#define PHONEMEMEM
#endif

// a bunch of gymnastics necessary to put the phoneme list into PROGMEM
// on the arduino.

const char phon_EH3[] PHONEMEMEM = "EH3";
const char phon_EH2[] PHONEMEMEM = "EH2";
const char phon_EH1[] PHONEMEMEM = "EH1";
const char phon_PA0[] PHONEMEMEM = "PA0";
const char phon_DT[] PHONEMEMEM = "DT";
const char phon_A2[] PHONEMEMEM = "A2";
const char phon_A1[] PHONEMEMEM = "A1";
const char phon_ZH[] PHONEMEMEM = "ZH";
const char phon_AH2[] PHONEMEMEM = "AH2";
const char phon_I3[] PHONEMEMEM = "I3";
const char phon_I2[] PHONEMEMEM = "I2";
const char phon_I1[] PHONEMEMEM = "I1";
const char phon_M[] PHONEMEMEM = "M";
const char phon_N[] PHONEMEMEM = "N";
const char phon_B[] PHONEMEMEM = "B";
const char phon_V[] PHONEMEMEM = "V";
const char phon_CH[] PHONEMEMEM = "CH";
const char phon_SH[] PHONEMEMEM = "SH";
const char phon_Z[] PHONEMEMEM = "Z";
const char phon_AW1[] PHONEMEMEM = "AW1";
const char phon_NG[] PHONEMEMEM = "NG";
const char phon_AH1[] PHONEMEMEM = "AH1";
const char phon_OO1[] PHONEMEMEM = "OO1";
const char phon_OO[] PHONEMEMEM = "OO";
const char phon_L[] PHONEMEMEM = "L";
const char phon_K[] PHONEMEMEM = "K";
const char phon_J[] PHONEMEMEM = "J";
const char phon_H[] PHONEMEMEM = "H";
const char phon_G[] PHONEMEMEM = "G";
const char phon_F[] PHONEMEMEM = "F";
const char phon_D[] PHONEMEMEM = "D";
const char phon_S[] PHONEMEMEM = "S";
const char phon_A[] PHONEMEMEM = "A";
const char phon_AY[] PHONEMEMEM = "AY";
const char phon_Y1[] PHONEMEMEM = "Y1";
const char phon_UH3[] PHONEMEMEM = "UH3";
const char phon_AH[] PHONEMEMEM = "AH";
const char phon_P[] PHONEMEMEM = "P";
const char phon_O[] PHONEMEMEM = "O";
const char phon_I[] PHONEMEMEM = "I";
const char phon_U[] PHONEMEMEM = "U";
const char phon_Y[] PHONEMEMEM = "Y";
const char phon_T[] PHONEMEMEM = "T";
const char phon_R[] PHONEMEMEM = "R";
const char phon_E[] PHONEMEMEM = "E";
const char phon_W[] PHONEMEMEM = "W";
const char phon_AE[] PHONEMEMEM = "AE";
const char phon_AE1[] PHONEMEMEM = "AE1";
const char phon_AW2[] PHONEMEMEM = "AW2";
const char phon_UH2[] PHONEMEMEM = "UH2";
const char phon_UH1[] PHONEMEMEM = "UH1";
const char phon_UH[] PHONEMEMEM = "UH";
const char phon_O2[] PHONEMEMEM = "O2";
const char phon_O1[] PHONEMEMEM = "O1";
const char phon_IU[] PHONEMEMEM = "IU";
const char phon_U1[] PHONEMEMEM = "U1";
const char phon_THV[] PHONEMEMEM = "THV";
const char phon_TH[] PHONEMEMEM = "TH";
const char phon_ER[] PHONEMEMEM = "ER";
const char phon_EH[] PHONEMEMEM = "EH";
const char phon_E1[] PHONEMEMEM = "E1";
const char phon_AW[] PHONEMEMEM = "AW";
const char phon_PA1[] PHONEMEMEM = "PA1";
const char phon_STOP[] PHONEMEMEM = "STOP";

const char *const phoneme_table[64] PHONEMEMEM = {
    phon_EH3, // 00
    phon_EH2, // 01
    phon_EH1, // 02
    phon_PA0, // 03
    phon_DT,  // 04
    phon_A2,  // 05
    phon_A1,  // 06
    phon_ZH,  // 07
    phon_AH2, // 08
    phon_I3,  // 09
    phon_I2,  // 0A
    phon_I1,  // 0B
    phon_M,   // 0C
    phon_N,   // 0D
    phon_B,   // 0E
    phon_V,   // 0F
    phon_CH,  // 10
    phon_SH,  // 11
    phon_Z,   // 12
    phon_AW1, // 13
    phon_NG,  // 14
    phon_AH1, // 15
    phon_OO1, // 16
    phon_OO,  // 17
    phon_L,   // 18
    phon_K,   // 19
    phon_J,   // 1A
    phon_H,   // 1B
    phon_G,   // 1C
    phon_F,   // 1D
    phon_D,   // 1E
    phon_S,   // 1F
    phon_A,   // 20
    phon_AY,  // 21
    phon_Y1,  // 22
    phon_UH3, // 23
    phon_AH,  // 24
    phon_P,   // 25
    phon_O,   // 26
    phon_I,   // 27
    phon_U,   // 28
    phon_Y,   // 29
    phon_T,   // 2A
    phon_R,   // 2B
    phon_E,   // 2C
    phon_W,   // 2D
    phon_AE,  // 2E
    phon_AE1, // 2F
    phon_AW2, // 30
    phon_UH2, // 31
    phon_UH1, // 32
    phon_UH,  // 33
    phon_O2,  // 34
    phon_O1,  // 35
    phon_IU,  // 36
    phon_U1,  // 37
    phon_THV, // 38
    phon_TH,  // 39
    phon_ER,  // 3A
    phon_EH,  // 3B
    phon_E1,  // 3C
    phon_AW,  // 3D
    phon_PA1, // 3E
    phon_STOP // 3F
};

bool getPhoneme(uint8_t phoneme, char *dest)
{
    if ((phoneme>=0) && (phoneme<=0x3F)) {
#ifdef ARDUINO
        strcpy_P(dest, (char *)pgm_read_word(&(phoneme_table[phoneme])));
#else
        strcpy(dest, phoneme_table[phoneme]);
#endif
        return true;
    } else {
        *dest = '\0';
        return false;
    }
}
