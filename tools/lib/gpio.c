/* Author: Tobi Vollebregt */

#include "gpio.h"
#include <string.h>

#define ARRAY_SIZE(x) ((int)(sizeof(x) / sizeof(x[0])))

/* Names taken from arch/arm/mach-lpc32xx/gpiolib.c */
static const char *const gpio_names[] = {
	"p0.0",
	"p0.1",
	"p0.2",
	"p0.3",
	"p0.4",
	"p0.5",
	"p0.6",
	"p0.7",
	"p1.0",
	"p1.1",
	"p1.2",
	"p1.3",
	"p1.4",
	"p1.5",
	"p1.6",
	"p1.7",
	"p1.8",
	"p1.9",
	"p1.10",
	"p1.11",
	"p1.12",
	"p1.13",
	"p1.14",
	"p1.15",
	"p1.16",
	"p1.17",
	"p1.18",
	"p1.19",
	"p1.20",
	"p1.21",
	"p1.22",
	"p1.23",
	"p2.0",
	"p2.1",
	"p2.2",
	"p2.3",
	"p2.4",
	"p2.5",
	"p2.6",
	"p2.7",
	"p2.8",
	"p2.9",
	"p2.10",
	"p2.11",
	"p2.12",
	"gpio00",
	"gpio01",
	"gpio02",
	"gpio03",
	"gpio04",
	"gpio05",
	"gpi00",
	"gpi01",
	"gpi02",
	"gpi03",
	"gpi04",
	"gpi05",
	"gpi06",
	"gpi07",
	"gpi08",
	"gpi09",
	"gpio61",
	"gpio62",
	"gpio63",
	"gpio64",
	"gpio65",
	"gpi15",
	"gpi16",
	"gpi17",
	"gpi18",
	"gpi19",
	"gpi20",
	"gpi21",
	"gpi22",
	"gpi23",
	"gpi24",
	"gpi25",
	"gpi26",
	"gpi27",
	"gpi28",
	"gpo00",
	"gpo01",
	"gpo02",
	"gpo03",
	"gpo04",
	"gpo05",
	"gpo06",
	"gpo07",
	"gpo08",
	"gpo09",
	"gpo10",
	"gpo11",
	"gpo12",
	"gpo13",
	"gpo14",
	"gpo15",
	"gpo16",
	"gpo17",
	"gpo18",
	"gpo19",
	"gpo20",
	"gpo21",
	"gpo22",
	"gpo23",
};

int gpio_name_to_idx(const char* name) {
	int i;

	for (i = 0; i < ARRAY_SIZE(gpio_names); ++i)
		if (strcmp(gpio_names[i], name) == 0)
			return i;

	return -1;
}

const char* gpio_idx_to_name(int idx) {
	if (idx < 0 || idx >= ARRAY_SIZE(gpio_names))
		return NULL;

	return gpio_names[idx];
}

#ifdef TEST
#include <stdio.h>

int main() {
	int idx = gpio_name_to_idx("p2.10");

	printf("p2.10 = %d\n", idx);
	printf("%d = %s\n", idx, gpio_idx_to_name(idx));

	printf("foo = %d\n", gpio_name_to_idx("foo"));
	printf("%d = %s\n", ARRAY_SIZE(gpio_names), gpio_idx_to_name(ARRAY_SIZE(gpio_names)));

	return 0;
}
#endif
