#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char*argv[]) 
{
  struct uproc up;
  int pid = 1;
  /* printf(1, "ps: Hello World. procinfo: %d\n", getprocinfo(12, &up)); */
  while((pid = getprocinfo(pid, &up)) > -1) {
    printf(1, "%d %s\n", up.pid, up.name);
  }
  exit();
}
