#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <fcntl.h>    
#include <sys/stat.h> 
#include "exchange.h"

#define PORT 8080

void handle_client(int client_socket) {
    char buffer[1024];
    char current_user_id[100] = "";
    int choice;

    while (1) {
        char *menu = "\n1.Register\n2.Login\n3.Deposit/Withdraw\n4.Exchange\n5.Toggle Shared/Individual\n6.Exit\nChoice: "; 
        send(client_socket, menu, strlen(menu), 0);
        
        memset(buffer, 0, 1024);
        int bytes = recv(client_socket, buffer, 1023, 0);
        if (bytes <= 0) break; 
        buffer[bytes] = '\0';
        choice = atoi(buffer);

        if (choice == 6) {
            close(client_socket);
            exit(0);
        }

        int sub_type = 0;
        int currency_idx = 0;
        int from_idx = 0; 
        int to_idx = 0;   
        double amount = 0;
        char temp_id[100] = "";

        if (choice == 1 || choice == 2) {
            send(client_socket, "Enter ID: ", 10, 0);
            int n = recv(client_socket, temp_id, 99, 0);
            temp_id[n] = '\0';
        } else if (choice == 3) {
            send(client_socket, "1.Deposit 2.Withdraw: ", 22, 0);
            recv(client_socket, buffer, 1023, 0); sub_type = atoi(buffer);
            send(client_socket, "Currency (0:EUR, 1:USD, 2:GBP): ", 32, 0);
            recv(client_socket, buffer, 1023, 0); currency_idx = atoi(buffer);
            send(client_socket, "Amount: ", 8, 0);
            recv(client_socket, buffer, 1023, 0); amount = atof(buffer);
        }else if (choice == 4) {
            send(client_socket, "From (0:EUR, 1:USD, 2:GBP): ", 28, 0);
            recv(client_socket, buffer, 1023, 0); from_idx = atoi(buffer);
            send(client_socket, "To (0:EUR, 1:USD, 2:GBP): ", 26, 0);
            recv(client_socket, buffer, 1023, 0); to_idx = atoi(buffer);
            send(client_socket, "Amount: ", 8, 0);
            recv(client_socket, buffer, 1023, 0); amount = atof(buffer);
        }

         int fd = open("db.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
      //  file_lock(fd, F_WRLCK); 
 
        struct UserAccount *users = NULL;
        int count = load_accounts(&users, "db.txt"); 
        
        switch(choice) {
            case 1: { // Register
                users = realloc(users, (count + 1) * sizeof(struct UserAccount));
                users[count].id = strdup(temp_id);
                users[count].is_shared = 0; 
                for(int i=0; i<3; i++) {
                    users[count].wallets[i].balance = 0;
                    strcpy(users[count].wallets[i].currency, (i==0)?"EUR":(i==1)?"USD":"GBP");
                }
                count++;
                quicksort_users(users, 0, count - 1);
                send(client_socket, "Account Created!\n", 17, 0);
                break;
            }
            case 2: { // Login
                int found = 0;
                for(int i=0; i < count; i++) {
                    if(strcmp(users[i].id, temp_id) == 0) {
                        strcpy(current_user_id, users[i].id);
                        found = 1; break;
                    }
                }
                send(client_socket, found ? "Login Success!\n" : "User Not Found!\n", 16, 0);
                break;
            }
            case 3: { // Deposit / Withdraw
                if (strlen(current_user_id) == 0) {
                    send(client_socket, "Login first!\n", 13, 0);
                } else {
                    for(int i=0; i<count; i++) {
                        if(strcmp(users[i].id, current_user_id) == 0) {
                            if (sub_type == 1) { // Deposit
                                users[i].wallets[currency_idx].balance += amount;
                               // send(client_socket, "Done!\n", 6, 0);
                            } else if (sub_type == 2 && users[i].wallets[currency_idx].balance >= amount) {
                                users[i].wallets[currency_idx].balance -= amount; 
                                send(client_socket, "Done!\n", 6, 0);
                            }// else {
                             //   send(client_socket, "Insufficient Funds!\n", 20, 0);
                            //}
                            break;
                        }
                    }
                }
                break;
            }
            case 4: { // Exchange
                 if (strlen(current_user_id) == 0) {
                    char *err = "Please login first!\n";
                    send(client_socket, err, strlen(err), 0);
                   // break;
                }else {
                    for(int i=0; i<count; i++) {
                        if(strcmp(users[i].id, current_user_id) == 0) {
                            if (from_idx >= 0 && to_idx >= 0 && users[i].wallets[from_idx].balance >= amount) {
                                double conv = amount * (rates[to_idx] / rates[from_idx]);
                                users[i].wallets[from_idx].balance -= amount;
                                users[i].wallets[to_idx].balance += conv;
                                send(client_socket, "Success!\n", 9, 0);
                            } else send(client_socket, "Failed!\n", 8, 0);
                            break;
                        }
                    }
                }
                break;
            }
            case 5: { // Toggle Shared
                for(int i=0; i<count; i++) {
                    if(strcmp(users[i].id, current_user_id) == 0) {
                        users[i].is_shared = !users[i].is_shared;
                        send(client_socket, "Status Changed!\n", 16, 0);
                        break;
                    }
                }
                break;
            }
        

            case 6: {
               printf("Client requested exit. Closing socket.\n");
               close(client_socket); 
               exit(0);              
               break;
            }
        }

    
        save_accounts_locked(users, count, "db.txt");
        file_lock(fd, F_UNLCK); 
        close(fd);

        for(int i=0; i<count; i++) free(users[i].id);
        free(users);
    }
}

int main() {
    int s_sock, c_sock;
    struct sockaddr_in addr;

    s_sock = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(s_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        return 1;
    }
    
    listen(s_sock, 10);
    printf("Server listening on port %d...\n", PORT);

    while(1) {
        c_sock = accept(s_sock, NULL, NULL);
        if(fork() == 0) { 
            close(s_sock);
            handle_client(c_sock);
            exit(0);
        }
        close(c_sock);
        waitpid(-1, NULL, WNOHANG); 
    }
    return 0;
}
