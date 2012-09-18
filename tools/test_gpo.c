/* Author: Tobi Vollebregt */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

void write_to_file(const char* filename, const char* data) {
	int fd = open(filename, O_WRONLY);
	if (fd < 0) {
		fputs("open: ", stderr);
		perror(filename);
		exit(EXIT_FAILURE);
	}
	if (write(fd, data, strlen(data)) < 0) {
		fputs("write: ", stderr);
		perror(filename);
		exit(EXIT_FAILURE);
	}
	if (close(fd) < 0) {
		fputs("close: ", stderr);
		perror(filename);
		exit(EXIT_FAILURE);
	}
}

int main() {
	int fd = open("/sys/class/gpio/gpo00/value", O_WRONLY);

	if (fd < 0) {
		fputs("open: ", stderr);
		perror("/sys/class/gpio/gpo00/value");
		exit(EXIT_FAILURE);
	}

	for (;;) {
		if (write(fd, "1", 1) < 0) {
			fputs("write: ", stderr);
			perror("/sys/class/gpio/gpo00/value");
			exit(EXIT_FAILURE);
		}
		usleep(1000000);
		if (write(fd, "0", 1) < 0) {
			fputs("write: ", stderr);
			perror("/sys/class/gpio/gpo00/value");
			exit(EXIT_FAILURE);
		}
		/*usleep(10);*/
	}

	return 0;
}
