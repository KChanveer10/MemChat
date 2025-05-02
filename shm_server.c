#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define SIZE 1024

struct shmseg {
    char usernames[SIZE];
    char messages[SIZE];
    char timestamp[SIZE];
    bool flag;
    bool readFlag;
    int numPro;
    int counter;
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
    int servcnt;

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

    memset(shm->messages,0,strlen(shm->messages));
    memset(shm->usernames,0,strlen(shm->usernames));
    memset(shm->timestamp,0,strlen(shm->timestamp));

    if (shm->usernames[0] == '\0') {  // If not initialized, do it
        shm->usernames[0] = '\0';
    }
    if (shm->messages[0] == '\0') {  // If not initialized, do it
        shm->messages[0] = '\0';
    }
    if (shm->timestamp[0] == '\0') {  // If not initialized, do it
        shm->timestamp[0] = '\0';
    }

    shm->flag=true;
    shm->flag=false;
    shm->counter=0;
    servcnt=0;



    printf("server is up\n");
    
        
    while (1)
    {
        sleep(2);
        printf("servcounter := %d\n",servcnt);
        printf("counter := %d\n",shm->counter);

        if(servcnt!=shm->counter){
            shm->readFlag=true;
        }
        servcnt=shm->counter;

        printf("Username := %s\n",shm->usernames);
        printf("Messages := %s\n",shm->messages);
        printf("Timestamp := %s\n",shm->timestamp);
        printf("ReadFlag := %d\n",shm->readFlag);


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