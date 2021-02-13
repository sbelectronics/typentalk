#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rawmode.h"
#include "../library/convert_stdio.h"
#include "../library/typetalk_stdio.h"

#define INPUTBUF_SIZE 750
char inputBuffer[INPUTBUF_SIZE];

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
  StdioTypeTalk *t = new StdioTypeTalk(c);
  int i;
  int quiet = false;

  t->initBuffer(inputBuffer, INPUTBUF_SIZE);

  for (i=1; i<argc; i++) {
    if (strcmp(argv[i], "--psend") == 0) {
      t->setPSend(true);
    } else if (strcmp(argv[i], "--noecho") == 0) {
      t->setEcho(false);
    } else if (strcmp(argv[i], "--caps") == 0) {
      t->setCaps(true);
    } else if (strcmp(argv[i], "--noemit") == 0) {
      c->SetEmit(false);
    } else if (strcmp(argv[i], "-q") == 0) {
      quiet = true;
    } else {
      fprintf(stderr, "unknown argument %s\n", argv[i]);
      exit(-1);
    }
  }

  if (!quiet) {
    fprintf(stderr, "PSend: %s\n", t->ModePSend ? "true": "false");
    fprintf(stderr, "Echo: %s\n", t->ModeEcho ? "true": "false");
    fprintf(stderr, "Caps: %s\n", t->ModeCaps ? "true": "false");
    fprintf(stderr, "Emit: %s\n", c->ModeEmit ? "true": "false");
    fprintf(stderr, "This program will not exit unless it gets an EOF. You may have to kill it.\n");
  }
  GoRawMode();

  while (true) {
    char c = getc(stdin);
    if (c == EOF) {
      return 0;
    }
    t->bufferCharacter(c);
  }

  return 0;
}
