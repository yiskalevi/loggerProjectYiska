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
#define MAX_LINE_LENGTH 64

void sent_msg(const char *msg);
int count_number_of_lines(const char *filename);
void info_comand(const char *filename, int num_line);
void log_comand(const char *filename, char *buffer);

socklen_t len;
int sockfd;
struct sockaddr_in servaddr, cliaddr;

int main()
{
	char buffer[MAXLINE];
	const char *filename = "log.txt";
	int len_msg_recive;
	char command[4];
	int num_line;
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

	len = sizeof(cliaddr); //len is value/result
	while (true) {
		len_msg_recive = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
		buffer[len_msg_recive] = '\0';
		if (len_msg_recive>64 ){
			sent_msg("Message length should be less than or equal to 64 bytes\n");
			continue;
		}
		printf("Client:%s\n", buffer);
		if (strncmp(buffer, "log ", 4) == 0) {
			log_comand(filename, buffer);
		} else if (strncmp(buffer, "info ", 5) == 0 && sscanf(buffer, "%s %d", command, &num_line) == 2) {
			info_comand(filename, num_line);
			continue;
		} else {
			sent_msg(
				"There are two possible actions:\n1. log: in the format of log .....\n2. info: in the format of info n (n is number between 0-8)\n");
		}
	}
	return 0;
}
void sent_msg(const char *msg)
{
	sendto(sockfd, msg, strlen(msg), MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);
	printf("message sent.\n");
}
int count_number_of_lines(const char *filename)
{
	FILE *fp;
	int count = 0; // Line counter (result)
	char c; // To store a character read from file
	// Open the file
	fp = fopen(filename, "r");
	// Check if file exists
	if (fp == NULL) {
		printf("Could not open file %s", filename);
		return -1;
	}

	// Extract characters from file and store in character c
	for (c = getc(fp); c != EOF; c = getc(fp))
		if (c == '\n') // Increment count if this character is newline
			count = count + 1;
	// Close the file
	fclose(fp);
	printf("The file %s has %d lines\n ", filename, count);
	return count;
}
void log_comand(const char *filename, char *buffer)
{
	printf("Log message:%s\n", buffer + 4);
	FILE *fp = fopen(filename, "a");
	if (fp == NULL) {
		sent_msg("Error opening file\n");
		return;
	}
	fwrite(buffer + 4, 1, strlen(buffer) - 4, fp);
	fputc('\n', fp);
	fclose(fp);
	sent_msg("Log message received\n");
}
/*void info_comand(const char *filename, int num_line)
{
	char msg[MAXLINE] = "Info message recive\n";

	if (num_line <= 8) {
		int total_lines = count_number_of_lines(filename);
		if (total_lines == -1) {
			strcpy(msg, "Error opening file\n");
			sent_msg(msg);
			return;
		}
		printf("1");
		if (num_line > total_lines) {
			printf("2");
			snprintf(msg, MAXLINE, "Have only %d logs\n", total_lines);
			sent_msg(msg);
			return;
		} else {
			printf("3");
			FILE *file = fopen(filename, "r");
			if (file == NULL) {
				strcpy(msg, "Error opening file\n");
				sent_msg(msg);
				return;
			}

			int start_line = total_lines - num_line;
			int current_line = 0;
			char buffer[MAX_LINE_LENGTH];

			while (fgets(buffer, MAX_LINE_LENGTH, file)) {
				if (current_line >= start_line) {
					if (current_line == start_line) {
						msg[0] = '\0';
					}
					strncat(msg, buffer, MAX_LINE_LENGTH - strlen(msg) - 1);

				}
				current_line++;
			}

			fclose(file);
		}
	}
	else{
		sent_msg("number between 0-8\n");
	}
	
}*/
void info_comand(const char *filename, int num_line)
{
    char msg[MAXLINE] = "Info message received\n";

    if (num_line <= 8) {
        int total_lines = count_number_of_lines(filename);
        if (total_lines == -1) {
            strcpy(msg, "Error opening file\n");
            sent_msg(msg);
            return;
        }
        if (num_line > total_lines) {
            snprintf(msg, MAXLINE, "Have only %d logs\n", total_lines);
            sent_msg(msg);
            return;
        } else {
            FILE *file = fopen(filename, "r");
            if (file == NULL) {
                strcpy(msg, "Error opening file\n");
                sent_msg(msg);
                return;
            }

            int start_line = total_lines - num_line;
            int current_line = 0;
            char buffer[MAX_LINE_LENGTH];

            // Reset msg to store the info message
            msg[0] = '\0';

            while (fgets(buffer, MAX_LINE_LENGTH, file)) {
                if (current_line >= start_line) {
                    strncat(msg, buffer, MAXLINE - strlen(msg) - 1);
                }
                current_line++;
            }

            fclose(file);
        }
    } else {
        strcpy(msg, "Number must be between 0 and 8\n");
    }

    sent_msg(msg);
}