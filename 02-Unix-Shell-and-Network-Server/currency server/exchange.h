#ifndef EXCHANGE_H
#define EXCHANGE_H

#include <stdio.h>
#include <fcntl.h>

#define MAX_CURRENCIES 3

struct Wallet {
    char currency[10];
    double balance;
};

struct UserAccount {
    char *id;
    struct Wallet wallets[MAX_CURRENCIES]; 
    int is_shared; // 1 για κοινό, 0 για ατομικό
};

extern double rates[3];


void file_lock(int fd, int type);
void save_accounts_locked(struct UserAccount *users, int count, const char *filename);
int load_accounts(struct UserAccount **users, const char *filename);
void quicksort_users(struct UserAccount users[], int left, int right);
int partition(struct UserAccount users[], int left, int right);

#endif
