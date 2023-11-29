#include "types.h"
#include "user.h"

int main(void)
{
    int retime, rutime, stime, pid;
    pid = fork();
    if (pid == 0){
        long long sum = 0;

        for(long long i = 0; i < (long long) 1e11; i++){
            sum += 1;
        }
        sleep(10);
        exit();
    }
    else{
        pid = wait2(&retime, &rutime, &stime);
        printf(1, "pid:%d retime:%d rutime:%d stime:%d\n", pid, retime, rutime, stime);
    }
    
	exit();
}