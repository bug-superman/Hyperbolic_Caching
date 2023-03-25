#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
int main() {
    struct timeval start, end;
    double time_used;

    gettimeofday(&start, NULL);
    usleep(50000);
    sleep(1);

     gettimeofday(&end, NULL);
    time_used = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    printf("代码运行时间为 %.2f 毫秒\n", time_used * 1000);
    return 0;
}