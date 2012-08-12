/* Author: Tobi Vollebregt */
#include "lib/fbutils/fbutils.h"
#include <stdio.h>

int main() {
	if (open_framebuffer() != 0)
		return 1;

	setcolor(0, 0);
	setcolor(1, 0xffffff);
	setcolor(2, 0xff0000);
	setcolor(3, 0x00ff00);
	setcolor(4, 0x0000ff);

	put_string(40, 40, "Hello, world!", 1);
	/* last column (239) is not visible */
	rect(0, 0, 238, 319, 2);
	fillrect(80, 80, 120, 120, 4);
	line(80, 80, 120, 120, 1);
	line(80, 120, 120, 80, 1);
	put_cross(200, 200, 3);

	getc(stdin);

	close_framebuffer();
	return 0;
}
