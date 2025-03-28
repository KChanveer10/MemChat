#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define SIZ 1024

struct shmseg
{
    char usernames[SIZ];
    char messgaes[SIZ];
    bool flag;
    int numPro;
};


void handle_signal(){
    int key = 5678;
    int shmid = shmget(key, sizeof(struct shmseg), IPC_CREAT | 0666);
    if (shmid < 0)
    {
        perror("shmget");
        exit(1);
    }

    if (shmctl(shmid, IPC_RMID, 0) == -1)
    {
        printf("shmid in the function is %d\n",shmid);
        perror("shmctl");
        exit(1);
    }
    printf("server is abruptly shutdown\n");
    exit(0);
}


void main()
{
    int shmid;
    key_t key;
    struct shmseg *shm;
    struct shmid_ds shm_info;

    /*
     * We'll name our shared memory segment
     * "5678".
     */

    key = 5678;

    /*
     * Create the segment.
     */
    //printf("%ld\n",sizeof(struct shmseg));

    shmid = shmget(key, sizeof(struct shmseg), IPC_CREAT | 0666);
    if (shmid < 0)
    {
        perror("shmget");
        exit(1);
    }


    signal(SIGINT,handle_signal);

    /*
     * Now we attach the segment to our data space.
     */
    if ((shm = shmat(shmid, NULL, 0)) == (void *)-1)
    {
        perror("shmat");
        exit(1);
    }

    if (shmctl(shmid, IPC_STAT, &shm_info) == -1) {
        perror("shmctl");
        exit(1);
    }


    /*
    If flag is false and numofusers (clients) are zero then shm seg should be removed
    */

    strcpy(shm->usernames, " ");
    strcpy(shm->messgaes, " ");
    shm->flag=true;


    printf("server is up\n");
    
    
    
    while (1)
    {
        sleep(3);
        printf("Username := %s\n",shm->usernames);
        printf("Messages := %s\n",shm->messgaes);

        // printf("Current mode: %o\n", shm_info.shm_perm.mode);

        // printf("creator of shm:- %d\n",shm_info.shm_cpid);
        // printf("last attached to shm:- %d\n",shm_info.shm_lpid);
        
        if (shm->flag==false && shm->numPro==1 )
        {
            if (shmdt(shm) == -1)
            {
                perror("shmdt");
                exit(1);
            }

            if (shmctl(shmid, IPC_RMID, 0) == -1)
            {
                perror("shmctl");
                exit(1);
            }
            break;
        }
    }
    
    printf("say bye to server\n");

    exit(0);
}