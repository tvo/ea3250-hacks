/* Author: Tobi Vollebregt */
#include <fcntl.h>
#include <stdio.h>

#define ADIN_PATH "/sys/devices/platform/lpc32xx-adc/adin2"

int main(int argc, char const* const* argv) {
	int i, fd = open(argc > 1 ? argv[1] : ADIN_PATH, O_RDONLY);
	int first_value = -1;

	if (fd < 0) {
		perror(ADIN_PATH ": open");
		return 1;
	}

	for (i = 0; i < 10000; ++i) {
		char buf[32];
		int value;

		if (pread(fd, buf, sizeof(buf), 0) < 0) {
			perror(ADIN_PATH ": read");
			close(fd);
			return 1;
		}

		value = atoi(buf);

		if (first_value == -1)
			first_value = value;

		if (abs(first_value - value) > 10)
			fputs("ERR\n", stdout);

		/*fputs(buf, stdout);
		fputc('\r', stdout);*/
	}

	close(fd);
	return 0;
}
