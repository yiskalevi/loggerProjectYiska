
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

void sent_msg(const char *msg);

socklen_t len;
int sockfd;
struct sockaddr_in servaddr, cliaddr;

int main()
{
	char buffer[MAXLINE];

	// Creating socket file descriptor
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));

	// Filling server information
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(PORT);

	// Bind the socket with the server address
	if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	int n;

	len = sizeof(cliaddr); //len is value/result
	while (true) {
		n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
		buffer[n] = '\0';
		printf("Client : %s\n", buffer);
		if (strncmp(buffer, "log ", 4) == 0) {
			printf("Log message: %s\n", buffer + 4);
			FILE *fp = fopen("log.txt", "a");
			if (fp == NULL) {
				perror("Error opening file");
				exit(EXIT_FAILURE);
			}
			fwrite(buffer + 4, 1, strlen(buffer) - 4, fp);
			fputc('\n', fp);
			fclose(fp);
		}
		elif (strncmp(buffer, "info ", 5) == 0)
		{
			char command[4];
			int n;
			if (sscanf(buffer, "%s %d", command, &n) == 2) {
				if (n <= 8) {
					//give n line
				} else {
					const char *msg = "The number must be positive and less than or equal to 8.\n";
					sent_msg(msg);
				}
			} else {
				const char *msg =
					"There are two possible actions:\n1. log: in the format of log .....\n2. info: in the format of info n (n is number between 0-8)\n";
				sent_msg(msg);
			}

			//sendto(sockfd, (const char *)hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);
			//printf("message sent.\n");
		}
		return 0;
	}
	void sent_msg(const char *msg)
	{
		sendto(sockfd, msg, strlen(msg), MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);
		printf("message sent.\n");
	}