#include "types.h"
#include "user.h"

int main(void)
{
    int retime, rutime, stime, pid;
    long long i, j;
    
    for (i = 0; i < 10; i++)
    {
        if (fork() == 0)
        {
            for (j = 1; j < (long long)1e10; j++){}

            sleep(2);
            exit();
        }
    }
    for (j = 0; j < 10; j++){
        pid = wait2(&retime, &rutime, &stime);
        printf(1, "pid:%d retime:%d rutime:%d stime:%d\n", pid, retime, rutime, stime);
    }



    exit();
}