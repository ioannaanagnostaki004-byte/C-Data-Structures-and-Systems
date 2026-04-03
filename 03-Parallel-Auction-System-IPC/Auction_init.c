#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <sys/stat.h>

#define SHM_KEY 12345
#define SEM_KEY 54321
#define MAX_BIDS 100

typedef struct {
    int highest_bid;
    int bidder_id;
    int bid_history[MAX_BIDS];
    int bid_count;
} auction_data_t;

void initialize_shared_memory() {
    int shm_id = shmget(SHM_KEY, sizeof(auction_data_t), IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (shm_id < 0) {
        perror("shmget");
        exit(1);
    }

    auction_data_t *data = (auction_data_t *)shmat(shm_id, NULL, 0);
    if (data == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    data->highest_bid = 0;
    data->bidder_id = -1;
    data->bid_count = 0;

    shmdt(data);

    int sem_id = semget(SEM_KEY, 2, IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (sem_id < 0) {
        perror("semget");
        exit(1);
    }

    semctl(sem_id, 0, SETVAL, 1); // Access semaphore
    semctl(sem_id, 1, SETVAL, 0); // Round semaphore

    printf("Initialization complete.\n");
}

int main() {
    initialize_shared_memory();
    return 0;
}

