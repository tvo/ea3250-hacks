/* Author: Tobi Vollebregt */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SYSFS_ADC                       "/sys/devices/platform/lpc32xx-adc/adin2"
#define DEV_ADC                         "/dev/adc"
#define NUM_SAMPLES                     1000000

int test_sysfs_interface() {
	int fd, i, sum = 0;

	fd = open(SYSFS_ADC, O_RDONLY);
	if (fd < 0) {
		perror(SYSFS_ADC ": open");
		return 1;
	}

	for (i = 0; i < NUM_SAMPLES; ++i) {
		char buf[16];

		if (pread(fd, buf, sizeof(buf), 0) < 0) {
			perror(SYSFS_ADC ": pread");
			close(fd);
			return 1;
		}

		sum += atoi(buf);
	}

	close(fd);

	printf("average sample value: %d\n", sum / NUM_SAMPLES);
	return 0;
}

int test_dev_adc_interface() {
	int fd, j, samples_read = 0, sum = 0, read_calls = 0;

	fd = open(DEV_ADC, O_RDONLY);
	if (fd < 0) {
		perror(DEV_ADC ": open");
		return 1;
	}

	while (samples_read < NUM_SAMPLES) {
		char buf[4096];
		int bytes_read;

		bytes_read = read(fd, buf, sizeof(buf));
		if (bytes_read < 0) {
			perror(DEV_ADC ": read");
			close(fd);
			return 1;
		}

		for (j = 0; j < bytes_read; j += 2)
			sum += *(short*)&buf[j];

		samples_read += bytes_read / 2;
		++read_calls;
	}

	close(fd);

	printf("actual number of samples: %d\n", samples_read);
	printf("average sample value: %d\n", sum / NUM_SAMPLES);
	printf("number of calls to read: %d\n", read_calls);
	printf("number of bytes per call: %d\n", samples_read * 2 / read_calls);
	return 0;
}

int main() {
	return /*test_sysfs_interface() ||*/ test_dev_adc_interface();
}
