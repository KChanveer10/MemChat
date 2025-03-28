/*
 * shm-client - client program to demonstrate shared memory.
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define SIZE 1024
struct shmseg{
    char usernames[SIZE];
    char messages[SIZE];
    bool flag;
    int numPro;
};

void main()
{
    int shmid;
    key_t key;
    char input[100];
    char messages[100];
    struct shmseg *shm;
    struct shmid_ds shm_info;

    /*
     * We need to get the segment named
     * "5678", created by the server.
     */

    key = 5678;

    /*
     * Locate the segment.
     */
    
    if ((shmid = shmget(key, sizeof(struct shmseg), 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    if ((shm = shmat(shmid, NULL, 0)) == (void *) -1) {
        perror("shmat");
        exit(1);
    }


    
    if (shmctl(shmid, IPC_STAT, &shm_info) == -1) {
        perror("shmctl");
        exit(1);
    }

    /*
     * Now we attach the segment to our data space.
     */


    printf("Enter the UserName: ");
    fgets(input, sizeof input, stdin); // <-- Crisis averted!
    input[strcspn(input, "\n")] = '\0';// <-- Necessary step.
    memset(shm->usernames,0,strlen(shm->usernames));
    strncpy(shm->usernames,input,strlen(input));

    shm->numPro = shm_info.shm_nattch;

    while(1){
        // printf("creator of shm:- %d\n",shm_info.shm_cpid);
        // printf("last attached to shm:- %d\n",shm_info.shm_lpid);
        // printf("Current mode: %o\n", shm_info.shm_perm.mode);
        //printf("number of process attached:- %ld\n",shm_info.shm_nattch);
        
        printf(">>>");
        fflush(stdout); 
        if (fgets(messages, sizeof(messages), stdin) == NULL) {
            // Handle error or EOF
            break;
        }
        messages[strcspn(messages, "\n")] = '\0'; // Remove newline
        
        // Append the message to shared memory
        memset(shm->messages,0,strlen(shm->messages));
        strncpy(shm->messages, messages, strlen(messages));
        
        // Check if the user wants to quit
        if (strcmp(messages, "q") == 0) {
            shm->numPro--;
            shm->flag=false;
            if (shmdt(shm) == -1)
            {
                perror("shmdt");
                exit(1);
            }
            break;
        }
    }

    exit(0);
}
