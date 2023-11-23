#include "types.h"
#include "stat.h"
#include "user.h"

const static char* states[] = { "UNUSED  ",
                                "EMBRYO  ",
                                "SLEEPING",
                                "RUNNABLE",
                                "RUNNING ",
                                "ZOMBIE  " };


int
main(int argc, char*argv[]) 
{
  struct uproc up;
  int pid = 1;


  while((pid = getprocinfo(pid, &up)) > -1) {
    printf(1, "\n");
    printf(1, "PID: %d\t", up.pid);
    printf(1, "NAME: %s\t", up.name);
    printf(1, "STATE: %s\t", states[up.state]);
    printf(1, "PPID: %d\t", up.ppid);
    printf(1, "SIZE: %dB\t", up.sz);
    printf(1, "\n");
  }
  exit();
}
