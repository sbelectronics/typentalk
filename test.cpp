#include <stdio.h>
#include <string.h>
#include "convert.h"

/* examples
 * MUSIC - CLbvwRKIY - PA0 - M - Y1 - IU - U1 - S - I1 - I3 - K
 * LATE - CXFaij - PA0 - L -  A1 - AY - Y - T
 */

int result = 0;

void test(const char *word, const char *phonemes)
{
  StringOutputConverter *sc = new StringOutputConverter();

  sc->convertString( (char*) word);
  
  if (strcmp(sc->outputBuf, phonemes) !=0) {
    fprintf(stderr, "ERROR word='%s', converted=`%s`, correct=`%s`\n", word, sc->outputBuf, phonemes);
    result++;
  } else {
    fprintf(stderr, "OK word='%s', converted=`%s`\n", word, sc->outputBuf);
  }

  delete sc;
}

#include "testcases.inc"

int main()
{
  execute_test_cases();

  if (result > 0) {
    fprintf(stderr, "*** %d errors ***\n");
  }

  return result;
}
