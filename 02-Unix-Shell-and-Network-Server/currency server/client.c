#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    char buffer[1024];

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    printf("Connected to Currency Exchange Server.\n");

    while(1) {
        memset(buffer, 0, sizeof(buffer));
        int n = recv(sock, buffer, sizeof(buffer) - 1, 0);
        
        if (n <= 0) {
            printf("\nDisconnected from server.\n");
            break;
        }

        buffer[n] = '\0';
        printf("%s", buffer);

        if (strstr(buffer, ":") != NULL) {
            char input[1024];
            if (fgets(input, sizeof(input), stdin)) {
                input[strcspn(input, "\n")] = 0; 
                
                send(sock, input, strlen(input), 0);
                
                if (strcmp(input, "6") == 0) {
                    printf("Exiting...\n");
                    break;
                }
            }
        }
    }

    close(sock);
    return 0;
}
