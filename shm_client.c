#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define SIZE 1024
// Shared variables
pthread_mutex_t cond_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int ready_to_read = 0;
char myUser[100];

struct shmseg
{
    char usernames[SIZE];
    char messages[SIZE];
    char timestamp[SIZE];
    bool flag;
    bool readFlag;
    int numPro;
    int counter;
};

char *getTimeInSecondsStr()
{
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    int seconds = local->tm_hour * 3600 + local->tm_min * 60 + local->tm_sec;

    char *buffer = malloc(16);
    if (buffer == NULL)
        return NULL;

    sprintf(buffer, "%d|", seconds);
    return buffer;
}

void split(char *str, const char *delim, char *parts[], int *count)
{
    char *token = strtok(str, delim);
    *count = 0;
    while (token != NULL && *count < SIZE)
    {
        parts[(*count)++] = token;
        token = strtok(NULL, delim);
    }
}

void DispStatus(struct shmseg *shmg)
{
    printf("\n--------------\n");
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

    for (int i = 0; i < count; i++)
    {
        printf("%s > %s\n", users[i], mesg[i]);
    }
    printf("------------------\n");
}

void *reader_thread(void *arg)
{
    struct shmseg *shm = (struct shmseg *)arg;

    pthread_mutex_lock(&cond_mutex);

    while (!ready_to_read)
    {
        pthread_cond_wait(&cond, &cond_mutex);
    }

    ready_to_read = 0;
    pthread_mutex_unlock(&cond_mutex);

    while (1)
    {
        if (shm->readFlag)
        {
            DispStatus(shm);
            printf("%s >>> ", myUser);
            fflush(stdout);
            shm->readFlag = false;
        }
    }
    return NULL;
}

void main()
{
    int shmid;
    key_t key;
    char input[100];
    char messages[100];
    struct shmseg *shm;
    struct shmid_ds shm_info;

    key = 5678;

    if ((shmid = shmget(key, sizeof(struct shmseg), 0666)) < 0)
    {
        printf("Server is not up yet! Sorry :(\n");
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

    pthread_t tid;
    pthread_create(&tid, NULL, reader_thread, (void *)shm);

    printf("Enter the UserName and Msg: ");
    fgets(input, sizeof input, stdin);
    input[strcspn(input, "\n")] = '\0';
    strncpy(myUser, input, sizeof(input) - 1);
    strncat(input, "|", sizeof(input) - strlen(input) - 1);

    shm->numPro = shm_info.shm_nattch;
    printf(">>> ");
    while (1)
    {
        pthread_mutex_lock(&cond_mutex);
        ready_to_read = 1;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&cond_mutex);

        if (fgets(messages, sizeof(messages), stdin) == NULL)
        {
            break;
        }
        messages[strcspn(messages, "\n")] = '\0';
        strncat(messages, "|", sizeof(messages) - strlen(messages) - 1);

        if (strcmp(messages, "q|") == 0)
        {
            strncat(shm->usernames, input, sizeof(shm->usernames) - strlen(shm->usernames) - 1);
            strncat(shm->messages, "Bye|", sizeof(shm->messages) - strlen(shm->messages) - 1);
            char *timeStr = getTimeInSecondsStr();
            strncat(shm->timestamp, timeStr, sizeof(shm->timestamp) - strlen(shm->timestamp) - 1);
            free(timeStr);
            shm->counter++;
            shm->numPro--;
            shm->flag = false;
            if (shmdt(shm) == -1)
            {
                perror("shmdt");
                exit(1);
            }
            break;
        }

        strncat(shm->usernames, input, sizeof(shm->usernames) - strlen(shm->usernames) - 1);
        strncat(shm->messages, messages, sizeof(shm->messages) - strlen(shm->messages) - 1);
        char *timeStr = getTimeInSecondsStr();
        strncat(shm->timestamp, timeStr, sizeof(shm->timestamp) - strlen(shm->timestamp) - 1);
        free(timeStr);
        shm->counter++;
        sleep(2);
    }
    exit(0);
}
