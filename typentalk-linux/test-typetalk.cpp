#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../library/convert_stdio.h"
#include "../library/typetalk_stdio.h"

int result = 0;

#define MAX_LEN 4096
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
  fflush(stdout);

  read(out_pipe[0], buffer, maxLen); /* read from pipe into buffer */

  dup2(saved_stdout, STDOUT_FILENO);  /* reconnect stdout for testing */
}

void test(const char *input, const char *output)
{
  StdioConverter *sc = new StdioConverter();
  StdioTypeTalk *tt = new StdioTypeTalk(sc);
  char buffer[MAX_LEN+1] = {0};

  sc->SetEmit(false);
  tt->setPSend(true);
  tt->setEcho(false);
  start_capture();

  for (const char *x=input; *x; x++) {
    tt->handleCharacter(*x);
  }
  tt->handleCharacter(0x10); // newline

  buffer[0] = 'C';
  end_capture(&buffer[1], MAX_LEN);

  if (strcmp(buffer, output) !=0) {
    fprintf(stderr, "ERROR word='%s', converted=`%s`, correct=`%s`\n", input, buffer, output);
    result++;
  } else {
    fprintf(stderr, "OK word='%s', converted=`%s`\n", input, output);
  }

  delete sc;
}

int main()
{
  test("this is a test of the emergency broadcast system", "CxIJ_~~KIRFaijB@_j~rcOxrB@Lz^ZB@M_iNkuw^Yo@_j_K_jB@L");
  test("now is the time for all good men to come to the aid of their country", "CMUcw~~KIRxrjU@iL~]utk}X\\VV^LB@M~jvwYrcL~jvwxrFai^~rcOx@E@kYUcmMjki");
  test("the quick brown fox jumped over the lazy dog", "CxrYmKIYNkUcwM]UcYC_^ZcqLejutOzxrXFaiRi^}\\");

  if (result > 0) {
    fprintf(stderr, "*** %d errors ***\n", result);
  }

  return result;
}
