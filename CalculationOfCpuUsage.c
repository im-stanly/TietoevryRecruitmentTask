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

void getCpuTime(CpuData *cpuData) {
    FILE* file = fopen("/proc/stat", "r");
    if (file == NULL) {
        perror("There was a problem opening the file");
        return 0;
    }

    char buffer[1024];

    char* ret = fgets(buffer, sizeof(buffer) - 1, file);
    if (ret == NULL) {
        perror("There was a problem starting to read /proc/stat");
        fclose(file);
        return;
    }
    fclose(file);

    sscanf(buffer,
        "cpu  %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu",
        &(cpuData->user), &(cpuData->nice), &(cpuData->system), &(cpuData->idle),
        &(cpuData->iowait), &(cpuData->irq), &(cpuData->softirq), &(cpuData->steal),
        &(cpuData->guest), &(cpuData->guest_nice));

    return;
}

int main(){
    return 0;
}