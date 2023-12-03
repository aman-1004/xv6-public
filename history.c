#include "types.h"
#include "user.h"



int
main(int argc, char*argv[]) 
{
  char buf[129];
  int next=0;
  int sr = 0;
  while((next = history(next, buf)) >= 0) {
    printf(1, "%d %s\n", ++sr, buf);
  };
  exit();
}
