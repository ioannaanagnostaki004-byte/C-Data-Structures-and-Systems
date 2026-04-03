#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "exchange.h"

double rates[3] = {1.0, 1.10, 0.85}; 

void file_lock(int fd, int type) {
    struct flock lock;
    lock.l_type = type;    // F_WRLCK για γράψιμο/διάβασμα, F_UNLCK για ξεκλείδωμα
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    fcntl(fd, F_SETLKW, &lock); 
}

void save_accounts_locked(struct UserAccount *users, int count, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) return;

    for (int i = 0; i < count; i++) {
        fprintf(file, "ID:%s Shared:%d\n", users[i].id, users[i].is_shared);
        for (int j = 0; j < MAX_CURRENCIES; j++) {
            fprintf(file, "%s %.2f\n", users[i].wallets[j].currency, users[i].wallets[j].balance);
        }
    }
    fclose(file);
}

int load_accounts(struct UserAccount **users, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) return 0;

    int count = 0;
    char id_buf[100];
    int shared;


    while (fscanf(file, "ID:%s Shared:%d\n", id_buf, &shared) == 2) {
        *users = realloc(*users, (count + 1) * sizeof(struct UserAccount));
        (*users)[count].id = strdup(id_buf);
        (*users)[count].is_shared = shared;

        for (int j = 0; j < MAX_CURRENCIES; j++) {
            fscanf(file, "%s %lf\n", (*users)[count].wallets[j].currency, 
                                     &(*users)[count].wallets[j].balance);
        }
        count++;
    }

    fclose(file);
    return count;
}


void quicksort_users(struct UserAccount users[], int left, int right) {
    if (left < right) {
        int pivotindex = partition(users, left, right);
        quicksort_users(users, left, pivotindex - 1);
        quicksort_users(users, pivotindex + 1, right);
    }
}

int partition(struct UserAccount users[], int left, int right) {
    struct UserAccount pivot = users[right];
    int i = left - 1;
    for (int j = left; j < right; j++) {
        if (strcmp(users[j].id, pivot.id) <= 0) {
            i++;
            struct UserAccount temp = users[i];
            users[i] = users[j];
            users[j] = temp;
        }
    }
    struct UserAccount temp = users[i + 1];
    users[i + 1] = users[right];
    users[right] = temp;
    return i + 1;
}
