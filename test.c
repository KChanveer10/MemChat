#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main() {
    int shmid = 32769;  // Replace with your SHMID
    struct shmid_ds shm_info;

    if (shmctl(shmid, IPC_STAT, &shm_info) == -1) {
        perror("shmctl");
        return 1;
    }

    printf("Creator PID (cpid): %d\n", shm_info.shm_cpid);
    printf("Last PID (lpid): %d\n", shm_info.shm_lpid);
    printf("Current attached processes: %ld\n", shm_info.shm_nattch);

    return 0;
}