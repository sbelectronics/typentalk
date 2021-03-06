#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rawmode.h"
#include "../library/convert_stdio.h"
#include "../library/typetalk_stdio.h"

/* rules for Z
   DA BF, DA         - Z E1 AY Y
   BF, 12            - Z
*/

/* possible a matches,

   0xDA, 0xFB, 0xBF, 0xFC, 0xE1, 0x06, 0x21, 0x29,
   0x82, 0x06, 0x21, 0x29

   mistmatched 0xBF, 0xFC, 0xF4, 0x06, 0x09, 0x21


/* examples
 * MUSIC - CLbvwRKIY - PA0 - M - Y1 - IU - U1 - S - I1 - I3 - K
 * LATE - CXFaij - PA0 - L -  A1 - AY - Y - T
 * Z - Z E1 AY Y
 */


int main(int argc, char **argv)
{
  StdioConverter *c = new StdioConverter();

  c->setDebug(true);
  strcpy(c->getWordBuf(), (char*) "emergency");
  c->convertBuffer();

  return 0;
}
