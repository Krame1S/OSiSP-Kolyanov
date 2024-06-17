#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

void interact_with_server(int server_socket) {
    char buffer[BUFFER_SIZE];

    while (1) {
        printf("> ");
        memset(buffer, 0, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        if (strncmp(buffer, "@", 1) == 0) {
            FILE *file = fopen(buffer + 1, "r");
            if (!file) {
                perror("fopen");
                continue;
            }
            while (fgets(buffer, BUFFER_SIZE, file)) {
                buffer[strlen(buffer) - 1] = 0;
                write(server_socket, buffer, strlen(buffer));
                memset(buffer, 0, BUFFER_SIZE);
                read(server_socket, buffer, BUFFER_SIZE);
                printf("%s\n", buffer);
            }
            fclose(file);
        } else {
            write(server_socket, buffer, strlen(buffer));
            memset(buffer, 0, BUFFER_SIZE);
            read(server_socket, buffer, BUFFER_SIZE);
            printf("%s\n", buffer);
        }

        if (strncmp(buffer, "BYE", 3) == 0) {
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1) 
    {
        perror("Error opening socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2])); // port

    int res = inet_pton(AF_INET, argv[1], &addr.sin_addr);
    if (res == 0) {
        printf("inet_pton failed");
        exit(EXIT_FAILURE);
    }
    if (res == -1) {
        perror("inet_pton failed");
        exit(EXIT_FAILURE);
    }

    if (connect(sfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) 
    {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server.\n");
    interact_with_server(sfd);

    close(sfd);
    return 0;
}