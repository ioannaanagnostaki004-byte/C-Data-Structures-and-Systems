#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <time.h>
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

void semaphore_wait(int sem_id, int sem_num) {
    struct sembuf op = {sem_num, -1, 0};
    semop(sem_id, &op, 1);
}

void semaphore_signal(int sem_id, int sem_num) {
    struct sembuf op = {sem_num, 1, 0};
    semop(sem_id, &op, 1);
}

void bidder(int id) {
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

    int sem_id = semget(SEM_KEY, 2, IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (sem_id < 0) {
        perror("semget");
        exit(1);
    }

    srand(time(NULL) + id);

    while (1) {
        semaphore_wait(sem_id, 1); // Wait for new round

        int bid = (rand() % 100) + 1;
        sleep((rand() % 3) + 1);

        if (bid > data->highest_bid) {
            data->highest_bid = bid;
            data->bidder_id = id;
            data->bid_history[data->bid_count++] = bid;
            printf("Bidder %d placed a new highest bid of %d\n", id, bid);
        } else {
            printf("Bidder %d's bid of %d was too low\n", id, bid);
        }

        semaphore_signal(sem_id, 0); // Signal auctioneer
    }

    shmdt(data);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <bidder_id>\n", argv[0]);
        exit(1);
    }

    int id = atoi(argv[1]);
    bidder(id);
    return 0;
}

