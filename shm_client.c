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
#include <time.h>

#define SIZE 1024

struct shmseg
{
    char usernames[SIZE];
    char messages[SIZE];
    char timestamp[SIZE];
    bool flag;
    int numPro;
};

char *getTimeInSecondsStr()
{
    time_t now = time(NULL);
    struct tm *local = localtime(&now);

    int seconds = local->tm_hour * 3600 + local->tm_min * 60 + local->tm_sec;

    // Allocate memory for the string (enough for up to 5-6 digit seconds + null terminator)
    char *buffer = malloc(16);
    if (buffer == NULL)
        return NULL;

    sprintf(buffer, "%d|", seconds);
    return buffer;
}

void split(char *str, const char *delim, char *parts[], int *count) {
    char *token = strtok(str, delim);
    *count = 0;
    while (token != NULL && *count < SIZE) {
        parts[(*count)++] = token;
        token = strtok(NULL, delim);
    }
}


void DispStatus(struct shmseg *shmg){
    printf("--------------\n");
    char user[SIZE];
    char messages[SIZE];

    strncpy(user, shmg->usernames, SIZE);
    user[SIZE - 1] = '\0'; // Ensure null termination

    strncpy(messages, shmg->messages, SIZE);
    messages[SIZE - 1] = '\0'; // Ensure null termination

    int user_count, msg_count;
    char *users[SIZE];
    char *mesg[SIZE];

    split(user, "|", users, &user_count);
    split(messages, "|", mesg, &msg_count);

    int count = (user_count < msg_count) ? user_count : msg_count;

    for (int i = 0; i < count; i++) {
        printf("%s > %s\n", users[i], mesg[i]);
    }
    printf("------------------\n");
}







void main()
{
    int shmid;
    key_t key;
    char input[100];
    char messages[100];
    char myUser[100];
    struct shmseg *shm;
    struct shmid_ds shm_info;
    int seconds;

    /*
     * We need to get the segment named
     * "5678", created by the server.
     */

    key = 5678;

    /*
     * Locate the segment.
     */

    if ((shmid = shmget(key, sizeof(struct shmseg), 0666)) < 0)
    {
        perror("shmget");
        exit(1);
    }

    if ((shm = shmat(shmid, NULL, 0)) == (void *)-1)
    {
        perror("shmat");
        exit(1);
    }

    if (shmctl(shmid, IPC_STAT, &shm_info) == -1)
    {
        perror("shmctl");
        exit(1);
    }

    /*
     * Now we attach the segment to our data space.
     */

    printf("Enter the UserName: ");
    fgets(input, sizeof input, stdin);  // <-- Crisis averted!
    input[strcspn(input, "\n")] = '\0'; // <-- Necessary step.
    strncpy(myUser, input, sizeof(input) - 1);
    strncat(input, "|", sizeof(input) - strlen(input) - 1);

    shm->numPro = shm_info.shm_nattch;

    while (1)
    {
        // printf("creator of shm:- %d\n",shm_info.shm_cpid);
        // printf("last attached to shm:- %d\n",shm_info.shm_lpid);
        // printf("Current mode: %o\n", shm_info.shm_perm.mode);
        // printf("number of process attached:- %ld\n",shm_info.shm_nattch);
        // fflush(stdout);
        printf("%s >>> ", myUser);
        if (fgets(messages, sizeof(messages), stdin) == NULL)
        {
            // Handle error or EOF
            break;
        }
        messages[strcspn(messages, "\n")] = '\0'; // Remove newline
        strncat(messages, "|", sizeof(messages) - strlen(messages) - 1);

        // Check if the user wants to quit
        if (strcmp(messages, "q|") == 0)
        {
            strncat(shm->usernames, input, sizeof(shm->usernames) - strlen(shm->usernames) - 1);
            strncat(shm->messages, "Bye|", sizeof(shm->messages) - strlen(shm->messages) - 1);
            char *timeStr = getTimeInSecondsStr();
            strncat(shm->timestamp, timeStr, sizeof(shm->timestamp) - strlen(shm->timestamp) - 1);
            DispStatus(shm);
            shm->numPro--;
            shm->flag = false;
            if (shmdt(shm) == -1)
            {
                perror("shmdt");
                exit(1);
            }
            break;
        }

        // Append the message to shared memory
        strncat(shm->usernames, input, sizeof(shm->usernames) - strlen(shm->usernames) - 1);
        strncat(shm->messages, messages, sizeof(shm->messages) - strlen(shm->messages) - 1);
        char *timeStr = getTimeInSecondsStr();
        strncat(shm->timestamp, timeStr, sizeof(shm->timestamp) - strlen(shm->timestamp) - 1);
        DispStatus(shm);
    }
    exit(0);
}
