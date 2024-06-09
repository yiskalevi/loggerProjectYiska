#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define DEVICE "/dev/myDev"
#define BUFFER_SIZE 4096

int main() {
    int fd;
    ssize_t ret;
    char buffer[BUFFER_SIZE];

    fd = open(DEVICE, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }

    ret = read(fd, buffer, 14);
    if (ret < 0) {
        perror("Failed to read from the device");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("Read %zd bytes from the device: %.*s\n", ret, (int)ret, buffer);

    close(fd);
    return EXIT_SUCCESS;
}
