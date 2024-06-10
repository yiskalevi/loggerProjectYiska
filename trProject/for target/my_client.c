#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>

#define PORT 8080
#define MAXLINE 1024
#define MAXLINEINPUT 65

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        // Discard remaining characters
    }
}

int main() {
    int sockfd;
    char buffer[MAXLINE];
    char input[MAXLINEINPUT + 1];  // +1 for the null terminator
    struct sockaddr_in servaddr;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("11.0.0.2");;
    /*
        servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;
    */

    int n;
    socklen_t  len;

    while (true) {
        printf("Enter message: ");
        if (fgets(input, sizeof(input), stdin) != NULL) {
            size_t l = strlen(input);
            if (l > 0 && input[l - 1] == '\n') {
                input[l - 1] = '\0';
                l--;

            }

            // Clear input buffer if the input is too long
            if (l == MAXLINEINPUT) {
                clear_input_buffer();
            }

            // Send message to server
            sendto(sockfd, (const char *)input, strlen(input), MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr));

            // Receive message from server
            n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
            buffer[n] = '\0';
            printf("Server: %s\n", buffer);
        }
    }

    close(sockfd);
    return 0;
}
