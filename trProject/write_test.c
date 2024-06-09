#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define DEVICE "/dev/myDev"
#define BUFFER_SIZE 4096

int main() {
    int fd;
    ssize_t ret;
    char buffer[BUFFER_SIZE];
    const char *data = "Hello, Kernel!";

    fd = open(DEVICE, O_WRONLY) ;
    if (fd < 0) {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }

    memset(buffer, 0, BUFFER_SIZE);  // Initialize buffer to 0
    strncpy(buffer, data, BUFFER_SIZE - 1);  // Ensure null-termination
    ret = write(fd, buffer, strlen(data));
    if (ret < 0) {
        perror("Failed to write to the device");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("Wrote %zd bytes to the device\n", ret);

    close(fd);
    return EXIT_SUCCESS;
}
