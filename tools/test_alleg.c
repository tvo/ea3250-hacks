/* Author: Tobi Vollebregt */

#define DEBUGMODE
#include <allegro.h>
#include "lib/gpio/gpio.h"

void wait() {
	char buf[2];
	int fd;

	/* Export and open GPI_01 */
	fd = gpio_open("gpi01", O_RDONLY);
	if (fd < 0) {
		TRACE("gpio_open failed\n");
		exit(1);
	}

	/* Poll GPI_01 as long as it is pressed */
	do {
		rest(50);
		if (pread(fd, buf, sizeof(buf), 0) < 0) {
			TRACE("failed to read from /sys/class/gpio/gpi01/value\n");
			exit(1);
		}
	} while (buf[0] == '0');

	/* Poll GPI_01 until it is pressed */
	do {
		rest(50);
		if (pread(fd, buf, sizeof(buf), 0) < 0) {
			TRACE("failed to read from /sys/class/gpio/gpi01/value\n");
			exit(1);
		}
	} while (buf[0] == '1');

	/* Close and unexport GPI_01 */
	if (gpio_close("gpi01", fd) < 0) {
		TRACE("gpio_close failed\n");
		exit(1);
	}
}

int main() {
	int y;

	if (allegro_init() != 0)
		return 1;

	/* Without keyboard installed, Allegro segfaults in
	   the parent process after the fork in init_console(). */
	if (install_keyboard() != 0)
		return 2;

	if (install_timer() != 0)
		return 3;

	set_color_depth(16);

	if (set_gfx_mode(GFX_FBCON, 240, 320, 0, 0) != 0) {
		TRACE("set_gfx_mode failed: %s\n", allegro_error);
		set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
		return 4;
	}

	/* last column (239) is not visible */
	rect(screen, 0, 0, 238, 319, makecol(255, 0, 0));
	rectfill(screen, 40, 40, 80, 80, makecol(0, 0, 255));

	wait();

	for (y = 0; y < 320; ++y)
		hline(screen, 0, y, 238, makecol(0, y * 256 / 320, 0));

	wait();

	clear_bitmap(screen);
	return 0;
}
END_OF_MAIN()
