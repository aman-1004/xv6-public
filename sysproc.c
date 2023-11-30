#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "stat.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// MODIFICATION
int
sys_getprocinfo(void)
{
  int pid;
  struct uproc* up;
  if(argint(0, &pid) < 0)
    return -1;
  if(argptr(1, (void*) &up, sizeof(*up)) < 0) 
    return -1;

  extern struct {
    struct spinlock lock;
    struct proc proc[NPROC];
  } ptable;

  struct proc* p;
  if (pid < 1) return -1;
  int found = 0;

  for(p = ptable.proc; p && p < &ptable.proc[NPROC]; p++) {
    if (found) return p->pid;
    if(p->pid != pid) continue;

    // Copy the attributes from proc to uproc
    up->pid = p->pid; 
    int i;
    for(i=0; i<15 && p->name[i]; i++) {
      up->name[i] = p->name[i];
    }
    up->name[i] = 0;
    /* up->next = -1; */
    up->state = (int) p->state;
    up->ppid = 0;
    if (p->parent) up->ppid = p->parent->pid;
    up->sz = p->sz;
    found = 1;
  }
  if (found) return 0;
  return -1;
}


int sys_wait2(void) 
{
  int *retime, *rutime, *stime;
  if (argptr(0, (void*)&retime, sizeof(retime)) < 0)
    return -1;
  if (argptr(1, (void*)&rutime, sizeof(retime)) < 0)
    return -1;
  if (argptr(2, (void*)&stime, sizeof(stime)) < 0)
    return -1;
  return wait2(retime, rutime, stime);
}
