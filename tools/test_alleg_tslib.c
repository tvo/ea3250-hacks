/* Author: Tobi Vollebregt */

#define DEBUGMODE
#include <allegro.h>

int main() {
	if (allegro_init() != 0)
		return 1;

	/* Without keyboard installed, Allegro segfaults in
	   the parent process after the fork in init_console(). */
	if (install_keyboard() != 0)
		return 2;

	if (install_mouse() != 0)
		return 3;

	if (install_timer() != 0)
		return 4;

	set_color_depth(16);

	if (set_gfx_mode(GFX_FBCON, 240, 320, 0, 0) != 0) {
		TRACE("set_gfx_mode failed: %s\n", allegro_error);
		set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
		return 5;
	}

	clear_bitmap(screen);

	alert("Choose a button to quit!", NULL, NULL, "Button 1", "Button 2", 0, 0);

	clear_bitmap(screen);
	return 0;
}
END_OF_MAIN()
