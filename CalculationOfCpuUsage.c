#include<stdio.h>
#include <pthread.h>
#include <unistd.h>

typedef struct CpuData
{
    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
    unsigned long long steal;
    unsigned long long guest;
    unsigned long long guest_nice;
}CpuData;

int main(){
    return 0;
}