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

CpuData *prevReaded = NULL, *nowReaded = NULL;

void setDefaultValuesCpuData(CpuData *cpuData){
    cpuData->guest = 0;
    cpuData->guest_nice = 0;
    cpuData->idle = 0;
    cpuData->iowait = 0;
    cpuData->irq = 0;
    cpuData->nice = 0;
    cpuData->softirq = 0;
    cpuData->steal = 0;
    cpuData->system = 0;
    cpuData->user = 0;
}

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

void *reader(){
    ///Pierwszy  wątek  (Reader)  czyta  /proc/stat  i  wysyła  odczytany  ciąg  znaków  (jako  raw  data  lub  jako
    ///strukturę z polami odczytanymi z pliku np. idle) do wątku drugiego (Analyzer)

    setDefaultValuesCpuData(prevReaded);
    setDefaultValuesCpuData(nowReaded);

    return NULL;
}

void *analyzer(){
    ///Wątek drugi (Analyzer) przetwarza dane i wylicza zużycie procesa (wyrażone w %) dla każdego rdzenia
    ///procesora widocznego w /proc/stat i wysyła przetworzone dane (zużycie procesora wyrażone w % dla
    ///każdego rdzenia) do wątku trzeciego (Printer)

    long long PrevIdle = (prevReaded->idle) + (prevReaded->iowait);
    long long Idle = (nowReaded->idle) + (nowReaded->iowait);
    long long PrevNonIdle = (prevReaded->user) + (prevReaded->nice) + (prevReaded->system) + (prevReaded->irq) 
                            + (prevReaded->softirq) + (prevReaded->steal);
    long long NonIdle = (nowReaded->user) + (nowReaded->nice) + (nowReaded->system) + (nowReaded->irq) 
                        + (nowReaded->softirq) + (nowReaded->steal);

    long long totald = (Idle + NonIdle) - (PrevIdle + PrevNonIdle);

    float CPU_Percentage = (totald - (Idle - PrevIdle))/totald;

    return NULL;
}

void *printer(){
    ///Wątek  trzeci  (Printer)  drukuje  na  ekranie  w  sposób  sformatowany  (format  dowolny,  ważne  aby  był
    ///przejrzysty) średnie zużycie procesora co sekunde

}

void *watchdog(){
    /*Wątek czwarty (Watchdog) pilnuje aby program się nie zawiesił. Tzn jeśli wątki nie wyślą informacji
    przez 2 sekundy o tym, że pracują to program kończy działanie z odpowiednim komunikatem błędu*/

}

int main(){
    return 0;
}