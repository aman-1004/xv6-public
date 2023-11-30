#include "types.h"
#include "user.h"

int main(void)
{
    int retime, rutime, stime, pid;
    pid = fork();
    if (pid == 0){

        int sum = 0;
        for(long long i = 0; i < (long long) 1e15; i++){
            for(long long j = 0; j < (long long) 1e15; j++)
            {
                sum+=10;
                sum-=10;
            }
        }
        sleep(10);
    }
    else{
        
        pid = wait2(&retime, &rutime, &stime);
        printf(1, "pid:%d retime:%d rutime:%d stime:%d\n", pid, retime, rutime, stime);
    }
    
	exit();
}