#include "types.h"
#include "user.h"

#define MAX_HISTORY 16
#define INPUT_BUF 128
#define INPUT

char cmdFromHistory[INPUT_BUF];


void printHistory()
{
  int i,count=0;
  for (i = 0; i < MAX_HISTORY; i++)
  {
    // 0 == newest command == historyId (always)
    if (history(cmdFromHistory, i) == 0)
    { // this is the sys call
        count++;
        printf(1, "%d: %s\n", count, cmdFromHistory);
    }
  }
}


