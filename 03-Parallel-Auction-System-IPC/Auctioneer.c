#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
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

void manage_auction(int rounds, int round_time) {
    int shm_id = shmget(SHM_KEY, sizeof(auction_data_t), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (shm_id < 0) {
        perror("shmget");
        exit(1);
    }

    auction_data_t *data = (auction_data_t *)shmat(shm_id, NULL, 0);
    if (data == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    int sem_id = semget(SEM_KEY, 2, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (sem_id < 0) {
        perror("semget");
        exit(1);
    }

    for (int i = 0; i < rounds; i++) {
        printf("Starting round %d\n", i + 1);
        data->highest_bid = 0;
        data->bidder_id = -1;

        semaphore_signal(sem_id, 1); // Signal new round

        sleep(round_time);

        semaphore_wait(sem_id, 0); // Wait for bidders to finish

        printf("Round %d complete. Highest bid: %d by bidder %d\n",
               i + 1, data->highest_bid, data->bidder_id);
    }

    printf("Auction complete. Bid history:\n");
    for (int i = 0; i < data->bid_count; i++) {
        printf("Bid %d: %d\n", i + 1, data->bid_history[i]);
    }

    shmdt(data);
}

int main() {
    manage_auction(5, 10); // 5 rounds, 10 seconds each
    return 0;
}

