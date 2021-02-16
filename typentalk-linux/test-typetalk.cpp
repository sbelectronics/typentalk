#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../library/convert_stdio.h"
#include "../library/typetalk_stdio.h"

int result = 0;

#define INPUTBUF_SIZE 750
char inputBuffer[INPUTBUF_SIZE];

#define MAX_OUTBUF_LEN 4096
int out_pipe[2];
int saved_stdout;

void start_capture()
{
  saved_stdout = dup(STDOUT_FILENO);  /* save stdout for display later */

  if( pipe(out_pipe) != 0 ) {          /* make a pipe */
    exit(1);
  }

  dup2(out_pipe[1], STDOUT_FILENO);   /* redirect stdout to the pipe */
  close(out_pipe[1]);
}

void end_capture(char *buffer, int maxLen)
{
  int bRead;

  fflush(stdout);

  bRead = read(out_pipe[0], buffer, maxLen); /* read from pipe into buffer */

  for (int i=0; i<bRead; i++) {
    // strcmp is hard with nulls in the sequence
    if (buffer[i] == '\0') {
      buffer[i] = 0xFE;
    }
  }

  dup2(saved_stdout, STDOUT_FILENO);  /* reconnect stdout for testing */
}

void safeFprint(FILE * stream, char *bufin) {
    unsigned char *buf = (unsigned char *) bufin;
    int i;
    while (*buf) {
        if ((*buf >= 0 && *buf <=31 && *buf!='\n') || (*buf >= 127)) {
            fprintf(stream, "<%02X>", *buf);
        } else {
            fprintf(stream, "%c", *buf);
        }
        buf++;
    }
}


void test(const char *input, const char *output)
{
  StdioConverter *sc = new StdioConverter();
  StdioTypeTalk *tt = new StdioTypeTalk(sc);
  char outputBuffer[MAX_OUTBUF_LEN+1] = {0};
  char tmp[8192];

  tt->initBuffer(inputBuffer, INPUTBUF_SIZE);

  sc->SetEmit(false);
  start_capture();

  for (const char *x=input; *x; x++) {
    tt->bufferCharacter(*x);
  }
  tt->bufferCharacter(0x0D); // newline

  end_capture(outputBuffer, MAX_OUTBUF_LEN);

  if (strcmp(outputBuffer, output) !=0) {
    sprintf(tmp, "ERROR word='%s', converted=`%s`, correct=`%s`\n", input, outputBuffer, output);
    safeFprint(stderr, tmp);
    result++;
  } else {
    sprintf(tmp, "OK word='%s', converted=`%s`\n", input, output);
    safeFprint(stderr, tmp);
  }

  delete sc;
}

void testEscapeSequences()
{
  // note: null is mapped to FE for easy strcmp

  // test with psend default

  test("this is a test of the emergency broadcast system",
       "this is a test of the emergency broadcast system\r");

  // test with psend off

  test("\x1B\x12this is a test of the emergency broadcast system",
       "\x1B\xFEthis is a test of the emergency broadcast system\r");  

  // test with psend on

  test("\x1B\x11this is a test of the emergency broadcast system",
       "\x1B\xFEthis is a test of the emergency broadcast system\rCxIJ_~~KIRFaijB@_j~rcOxrB@Lz^ZB@M_iNkuw^Yo@_j_K_jB@L");

  test("\x1B\x11now is the time for all good men to come to the aid of their country", 
       "\x1B\xFEnow is the time for all good men to come to the aid of their country\rCMUcw~~KIRxrjU@iL~]utk}X\\VV^LB@M~jvwYrcL~jvwxrFai^~rcOx@E@kYUcmMjki");

  test("\x1B\x11the quick brown fox jumped over the lazy dog",
      "\x1B\xFEthe quick brown fox jumped over the lazy dog\rCxrYmKIYNkUcwM]UcYC_^ZcqLejutOzxrXFaiRi^}\\");

  // test with psend on, echo off

  test("\x1B\x11\x1B\x14this is a test of the emergency broadcast system",
       "\x1B\xFE\x1B" "CxIJ_~~KIRFaijB@_j~rcOxrB@Lz^ZB@M_iNkuw^Yo@_j_K_jB@L");  

  // test with psend on, echo on

  test("\x1B\x11\x1B\x13this is a test of the emergency broadcast system",
       "\x1B\xFE\x1B\xFEthis is a test of the emergency broadcast system\rCxIJ_~~KIRFaijB@_j~rcOxrB@Lz^ZB@M_iNkuw^Yo@_j_K_jB@L");

  // test ESC ESC

  test("\x1B\x1Bthis is a test of the emergency broadcast system",
       "\x1B\x1Bthis is a test of the emergency broadcast system\r");

  if (result > 0) {
    fprintf(stderr, "*** %d errors ***\n", result);
  }
}

void testPhrases()
{
  // test with psend on, echo off

  test("\x1B\x11\x1B\x14now is the time for all good men to come to the aid of their country", 
       "\x1B\xFE\x1B" "CMUcw~~KIRxrjU@iL~]utk}X\\VV^LB@M~jvwYrcL~jvwxrFai^~rcOx@E@kYUcmMjki");

  test("\x1B\x11\x1B\x14the quick brown fox jumped over the lazy dog",
      "\x1B\xFE\x1B" "CxrYmKIYNkUcwM]UcYC_^ZcqLejutOzxrXFaiRi^}\\");
}

void testOverflow()
{
  test("\x1B\x11\x1B\x14supercalifragilisticexpialidocious",
       "\x1B\xFE\x1B" "C_vwezYo@XKI]ko@^ZKXKI_jK_B@YC_eKIcXKIKI^ttQc_");

  test("\x1B\x11\x1B\x14" "abababababababababababababababababababab",
       "\x1B\xFE\x1B" "Co@No@No@No@No@No@No@No@No@No@No@No@No@Nqco@No@No@No@No@No@No@N");

  test("\x1B\x11\x1B\x14" "cabcabcabcabcabcabcabcabcabcabcabcabcabcab",
       "\x1B\xFE\x1B" "CYo@NYo@NYo@NYo@NYo@NYo@NYo@NYo@NYo@NNYo@NYo@NYo@NYo@NYo@N");

  test("\x1B\x11\x1B\x14" "dogcatdogcatdogcatdogcatdogcatdogcatdogcat",
       "\x1B\xFE\x1B" "C^}\\Yo@j^Uc\\Yo@j^Uc\\Yo@j^Uc\\Yo@j^Uc\\\\Yo@j^Uc\\Yo@j^Uc\\Yo@j"); 

  test("\x1B\x11\x1B\x14" "dogscatsdogscatsdogscatsdogscatsdogscatsdogs",
       "\x1B\xFE\x1B" "C^}\\_Yo@j_^Uc\\_Yo@j_^Uc\\_Yo@j_^Uc\\\\_Yo@j_^Uc\\_Yo@j_^Uc\\R"); 
}

int main()
{
  testEscapeSequences();
  testPhrases();
  testOverflow();

  return result;
}
