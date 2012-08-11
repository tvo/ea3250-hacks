/* Author: Tobi Vollebregt */

#define DEBUGMODE
#include <allegro.h>

void wait() {
	char buf[2];
	int fd;

	/* Export GPI_01 */
	fd = open("/sys/class/gpio/export", O_WRONLY | O_TRUNC);
	if (fd < 0) {
		TRACE("failed to open /sys/class/gpio/export\n");
		exit(1);
	}
	if (write(fd, "52\n", 3) < 0) {
		TRACE("failed to write to /sys/class/gpio/export\n");
		exit(1);
	}
	close(fd);

	/* Poll GPI_01 for button press */
	fd = open("/sys/class/gpio/gpi01/value", O_RDONLY);
	if (fd < 0) {
		TRACE("failed to open /sys/class/gpio/gpi01/value\n");
		exit(1);
	}

	/* Wait while it's still pressed */
	do {
		rest(50);
		if (pread(fd, buf, sizeof(buf), 0) < 0) {
			TRACE("failed to read from /sys/class/gpio/gpi01/value\n");
			exit(1);
		}
	} while (buf[0] == '0');

	/* Poll for next press */
	do {
		rest(50);
		if (pread(fd, buf, sizeof(buf), 0) < 0) {
			TRACE("failed to read from /sys/class/gpio/gpi01/value\n");
			exit(1);
		}
	} while (buf[0] == '1');

	close(fd);

	/* Unexport GPI_01 */
	fd = open("/sys/class/gpio/unexport", O_WRONLY | O_TRUNC);
	if (fd < 0) {
		TRACE("failed to open /sys/class/gpio/unexport\n");
		exit(1);
	}
	if (write(fd, "52\n", 3) < 0) {
		TRACE("failed to write to /sys/class/gpio/unexport\n");
		exit(1);
	}
	close(fd);
}

int main() {
	int y;

	if (allegro_init() != 0)
		return 1;

	if (install_timer() != 0)
		return 2;

	set_color_depth(16);

	if (set_gfx_mode(GFX_FBCON, 240, 320, 0, 0) != 0) {
		TRACE("set_gfx_mode failed: %s\n", allegro_error);
		set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
		return 3;
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
