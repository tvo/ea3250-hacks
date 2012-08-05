/* Author: Tobi Vollebregt */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>

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
	int fd = open("/sys/class/gpio/gpo17/value", O_WRONLY);
	int value = 0;
	char buf[16];

	if (fd < 0) {
		fputs("open: ", stderr);
		perror("/sys/class/gpio/gpo17/value");
		exit(EXIT_FAILURE);
	}

	for (;;) {
		value = 1 - value;
		//sprintf(buf, "%d", value);
		//write_to_file("/sys/class/gpio/gpo17/value", buf);
		if (write(fd, value ? "1" : "0", 1) < 0) {
			fputs("write: ", stderr);
			perror("/sys/class/gpio/gpo17/value");
			exit(EXIT_FAILURE);
		}
	}

	return 0;
}
