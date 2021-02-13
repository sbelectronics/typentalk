#include <iostream>
#include <termios.h>

#include "rawmode.h"

struct termios storedSettings;

void GoRawMode()
{
  struct termios newSettings;
  
  tcgetattr(0, &storedSettings);

  newSettings = storedSettings;
  newSettings.c_lflag &= (~ICANON);
  newSettings.c_lflag &= (~ECHO);
  newSettings.c_lflag &= (~ISIG);
  newSettings.c_cc[VTIME] = 1;
  newSettings.c_cc[VTIME] = 0;

  tcsetattr(0, TCSANOW, &newSettings);
}

void LeaveRawMode()
{
  tcsetattr(0, TCSANOW, &storedSettings);
}
