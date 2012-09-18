/*         ______   ___    ___
 *        /\  _  \ /\_ \  /\_ \
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      MIDI instrument playing test program for the Allegro library.
 *
 *      Original version by Shawn Hargreaves.
 *
 *      Adapted by Tobi Vollebregt to:
 *       - output midi signals on serial port,
 *       - fit the GUI on a 240x320 touch screen.
 */


#define DEBUGMODE
#include "allegro.h"
#include <linux/serial.h>
#include <sys/ioctl.h>
#include <termios.h>



extern DIALOG thedialog[];


#define INSTLIST     1
#define PIANO        2
#define VOLUME       3
#define PAN          4



#define MIDI_PORT    "/dev/ttyS1"
#define MIDI_BAUD    31250

int serial;



int serial_midi_init()
{
   struct termios termios;
   struct serial_struct serial_struct;

   serial = open(MIDI_PORT, O_WRONLY);
   if (serial < 0) {
      TRACE("open: %s\n", strerror(errno));
      return -1;
   }

   /* Source: /usr/lib/python2.6/dist-packages/serial/serialposix.py */
   if (tcgetattr(serial, &termios)) {
      TRACE("tcgetattr: %s\n", strerror(errno));
      close(serial);
      return -1;
   }

   /* set up raw mode / no echo / binary */
   termios.c_cflag |= CLOCAL | CREAD;
   termios.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG | IEXTEN);
#ifdef ECHOCTL
   termios.c_lflag &= ~ECHOCTL;
#endif
#ifdef ECHOKE
   termios.c_lflag &= ~ECHOKE;
#endif

   termios.c_oflag &= ~OPOST;
   termios.c_iflag &= ~(INLCR | IGNCR | ICRNL | IGNBRK);
#ifdef IUCLC
   termios.c_iflag &= ~IUCLC;
#endif
#ifdef PARMRK
   termios.c_iflag &= ~PARMRK;
#endif

   /* setup baud rate */
   cfsetispeed(&termios, B38400);
   cfsetospeed(&termios, B38400);

   /* setup char len */
   termios.c_cflag &= ~CSIZE;
   termios.c_cflag |= CS8;

   /* setup stopbits */
   termios.c_cflag &= ~CSTOPB;

   /* setup parity */
   termios.c_iflag &= ~(INPCK | ISTRIP);
   termios.c_cflag &= ~(PARENB | PARODD);

   /* setup flow control */
   termios.c_iflag &= ~(IXON | IXOFF | IXANY);
#if defined(CRTSCTS)
   termios.c_cflag &= ~CRTSCTS;
#elif defined(CNEW_RTSCTS)
   termios.c_cflag &= ~CNEW_RTSCTS;
#endif

   /* buffer */
   termios.c_cc[VMIN] = 0;
   termios.c_cc[VTIME] = 0;

   /* activate settings */
   if (tcsetattr(serial, TCSANOW, &termios)) {
      TRACE("tcsetattr: %s\n", strerror(errno));
      close(serial);
      return -1;
   }

   /* apply custom baud rate */
   if (ioctl(serial, TIOCGSERIAL, &serial_struct)) {
      TRACE("TIOCGSERIAL ioctl: %s\n", strerror(errno));
      close(serial);
      return -1;
   }

   serial_struct.custom_divisor = serial_struct.baud_base / MIDI_BAUD;
   serial_struct.flags &= ~ASYNC_SPD_MASK;
   serial_struct.flags |= ASYNC_SPD_CUST;

   if (ioctl(serial, TIOCSSERIAL, &serial_struct)) {
      TRACE("TIOCSSERIAL ioctl: %s\n", strerror(errno));
      close(serial);
      return -1;
   }

   return 0;
}



void serial_midi_out(unsigned char* msg, int size)
{
   if (write(serial, msg, size) < 0)
      TRACE("serial: %s\n", strerror(errno));
}



void set_patch(int channel, int prog)
{
   unsigned char msg[2];

   msg[0] = 0xC0+channel;
   msg[1] = prog;

   serial_midi_out(msg, 2);
}



void set_pan(int channel, int pan)
{
   unsigned char msg[3];

   msg[0] = 0xB0+channel;
   msg[1] = 10;
   msg[2] = pan / 2;

   serial_midi_out(msg, 3);
}



void note_on(int channel, int pitch, int vel)
{
   unsigned char msg[3];

   msg[0] = 0x90+channel;
   msg[1] = pitch;
   msg[2] = vel / 2;

   serial_midi_out(msg, 3);
}



void note_off(int channel, int pitch)
{
   unsigned char msg[3];

   msg[0] = 0x80+channel;
   msg[1] = pitch;
   msg[2] = 0;

   serial_midi_out(msg, 3);
}



char *instlist_getter(int index, int *list_size)
{
   static char *names[] =
   {
      "Acoustic Grand",
      "Bright Acoustic",
      "Electric Grand",
      "Honky-Tonk",
      "Electric Piano 1",
      "Electric Piano 2",
      "Harpsichord",
      "Clav",
      "Celesta",
      "Glockenspiel",
      "Music Box",
      "Vibraphone",
      "Marimba",
      "Xylophone",
      "Tubular Bells",
      "Dulcimer",
      "Drawbar Organ",
      "Percussive Organ",
      "Rock Organ",
      "Church Organ",
      "Reed Organ",
      "Accoridan",
      "Harmonica",
      "Tango Accordian",
      "Acoustic Guitar (nylon)",
      "Acoustic Guitar (steel)",
      "Electric Guitar (jazz)",
      "Electric Guitar (clean)",
      "Electric Guitar (muted)",
      "Overdriven Guitar",
      "Distortion Guitar",
      "Guitar Harmonics",
      "Acoustic Bass",
      "Electric Bass (finger)",
      "Electric Bass (pick)",
      "Fretless Bass",
      "Slap Bass 1",
      "Slap Bass 2",
      "Synth Bass 1",
      "Synth Bass 2",
      "Violin",
      "Viola",
      "Cello",
      "Contrabass",
      "Tremolo Strings",
      "Pizzicato Strings",
      "Orchestral Strings",
      "Timpani",
      "String Ensemble 1",
      "String Ensemble 2",
      "SynthStrings 1",
      "SynthStrings 2",
      "Choir Aahs",
      "Voice Oohs",
      "Synth Voice",
      "Orchestra Hit",
      "Trumpet",
      "Trombone",
      "Tuba",
      "Muted Trumpet",
      "French Horn",
      "Brass Section",
      "SynthBrass 1",
      "SynthBrass 2",
      "Soprano Sax",
      "Alto Sax",
      "Tenor Sax",
      "Baritone Sax",
      "Oboe",
      "English Horn",
      "Bassoon",
      "Clarinet",
      "Piccolo",
      "Flute",
      "Recorder",
      "Pan Flute",
      "Blown Bottle",
      "Skakuhachi",
      "Whistle",
      "Ocarina",
      "Lead 1 (square)",
      "Lead 2 (sawtooth)",
      "Lead 3 (calliope)",
      "Lead 4 (chiff)",
      "Lead 5 (charang)",
      "Lead 6 (voice)",
      "Lead 7 (fifths)",
      "Lead 8 (bass+lead)",
      "Pad 1 (new age)",
      "Pad 2 (warm)",
      "Pad 3 (polysynth)",
      "Pad 4 (choir)",
      "Pad 5 (bowed)",
      "Pad 6 (metallic)",
      "Pad 7 (halo)",
      "Pad 8 (sweep)",
      "FX 1 (rain)",
      "FX 2 (soundtrack)",
      "FX 3 (crystal)",
      "FX 4 (atmosphere)",
      "FX 5 (brightness)",
      "FX 6 (goblins)",
      "FX 7 (echoes)",
      "FX 8 (sci-fi)",
      "Sitar",
      "Banjo",
      "Shamisen",
      "Koto",
      "Kalimba",
      "Bagpipe",
      "Fiddle",
      "Shanai",
      "Tinkle Bell",
      "Agogo",
      "Steel Drums",
      "Woodblock",
      "Taiko Drum",
      "Melodic Tom",
      "Synth Drum",
      "Reverse Cymbal",
      "Guitar Fret Noise",
      "Breath Noise",
      "Seashore",
      "Bird Tweet",
      "Telephone ring",
      "Helicopter",
      "Applause",
      "Gunshot",
      "Acoustic Bass Drum",
      "Bass Drum 1",
      "Side Stick",
      "Acoustic Snare",
      "Hand Clap",
      "Electric Snare",
      "Low Floor Tom",
      "Closed Hi-Hat",
      "High Floor Tom",
      "Pedal Hi-Hat",
      "Low Tom",
      "Open Hi-Hat",
      "Low-Mid Tom",
      "Hi-Mid Tom",
      "Crash Cymbal 1",
      "High Tom",
      "Ride Cymbal 1",
      "Chinese Cymbal",
      "Ride Bell",
      "Tambourine",
      "Splash Cymbal",
      "Cowbell",
      "Crash Cymbal 2",
      "Vibraslap",
      "Ride Cymbal 2",
      "Hi Bongo",
      "Low Bongo",
      "Mute Hi Conga",
      "Open Hi Conga",
      "Low Conga",
      "High Timbale",
      "Low Timbale",
      "High Agogo",
      "Low Agogo",
      "Cabasa",
      "Maracas",
      "Short Whistle",
      "Long Whistle",
      "Short Guiro",
      "Long Guiro",
      "Claves",
      "Hi Wood Block",
      "Low Wood Block",
      "Mute Cuica",
      "Open Cuica",
      "Mute Triangle",
      "Open Triangle"
   };

   if (index < 0) {
      if (list_size)
	 *list_size = sizeof(names) / sizeof(char *);

      return NULL;
   }

   return names[index];
}



int instlist_proc(int msg, DIALOG *d, int c)
{
   int ret = d_list_proc(msg, d, c);

   if (ret & D_CLOSE) {
      ret &= ~D_CLOSE;
      object_message(thedialog+PIANO, MSG_KEY, 0);
   }

   return ret;
}



int piano_proc(int msg, DIALOG *d, int c)
{
/* Key width */
#define KW 10
/* Key color when pressed */
#define KC makecol(0, 255, 0)

   static char blackkey[12] =
   {
      FALSE, TRUE, FALSE, TRUE, FALSE, FALSE,
      TRUE, FALSE, TRUE, FALSE, TRUE, FALSE
   };

   static int playing_channel = -1;
   static int playing_pitch = -1;

   int channel = 0;
   int patch = 0;
   int pitch = 60;
   int delay = 140;
   int i, t, b;

   (void) c; /* unused */

   switch (msg) {

      case MSG_START:
	 d->d1 = -1;
	 d->d2 = -1;
	 break;

      case MSG_DRAW:
	 for (i=0; i<d->h/KW; i++) {
	    if (!blackkey[i%12]) {
	       t = i*KW;
	       b = (i+1)*KW;
	       if (blackkey[(i-1)%12])
		  t -= KW/2;
	       if (blackkey[(i+1)%12])
		  b += KW/2;
	       rectfill(screen, d->x+1, d->y+t+1, d->x+d->w-1, d->y+b-1, (i == d->d1) ? KC : d->bg);
	       rect(screen, d->x, d->y+t, d->x+d->w, d->y+b, d->fg);
	    }
	 }
	 for (i=0; i<d->h/KW; i++) {
	    if (blackkey[i%12]) {
	       t = i*KW + 1;
	       b = (i+1)*KW - 1;
	       rectfill(screen, d->x+d->w/4, d->y+t, d->x+d->w, d->y+b, (i == d->d1) ? KC : d->fg);
	    }
	 }
	 break;

      case MSG_CLICK:
	 d->d1 = (mouse_y - d->y) / KW;

	 set_clip_rect(screen, d->x, d->y+d->d1*KW-KW/2, d->x+d->w, d->y+d->d1*KW+KW+KW/2);
	 object_message(d, MSG_DRAW, 0);
	 set_clip_rect(screen, 0, 0, SCREEN_W-1, SCREEN_H-1);

	 pitch = 36 + d->d1;
	 delay = 0;
	 /* fallthrough */

      case MSG_KEY:
	 d->d2 = retrace_count+delay;

	 if (playing_channel >= 0)
	    note_off(playing_channel, playing_pitch);

	 patch = thedialog[INSTLIST].d1;

	 if (patch >= 128) {
	    channel = 9;
	    pitch = patch - 93;
	 }
	 else {
	    set_patch(channel, patch);
	 }

	 set_pan(channel, CLAMP(0, thedialog[PAN].d2, 255));
	 note_on(channel, pitch, CLAMP(0, thedialog[VOLUME].d2, 255));

	 playing_channel = channel;
	 playing_pitch = pitch;

	 do {
	    poll_mouse();
	 } while ((mouse_b) && (d->d1 == (mouse_y - d->y) / KW));

	 if (d->d1 >= 0) {
	    set_clip_rect(screen, d->x, d->y+d->d1*KW-KW/2, d->x+d->w, d->y+d->d1*KW+KW+KW/2);
	    d->d1 = -1;
	    object_message(d, MSG_DRAW, 0);
	    set_clip_rect(screen, 0, 0, SCREEN_W-1, SCREEN_H-1);
	 }
	 break;

      case MSG_IDLE:
	 if ((d->d2 >= 0) && (retrace_count > d->d2)) {
	    if (playing_channel >= 0) {
	       note_off(playing_channel, playing_pitch);
	       playing_channel = -1;
	       playing_pitch = -1;
	    }
	    d->d2 = -1;
	 }
	 break;
   }

   return D_O_K;

#undef KC
#undef KW
}



DIALOG thedialog[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key)    (flags)     (d1)           (d2)     (dp)              (dp2) (dp3) */
   { d_clear_proc,      0,    0,    0,    0,    0,    8,    0,       0,          0,             0,       NULL,             NULL, NULL  },
   { instlist_proc,     44,   4,   191,  256,   255,  0,    32,      D_EXIT,     0,             0,       instlist_getter,  NULL, NULL  },
   { piano_proc,        0,    0,    40,  320,   255,  0,    0,       0,          0,             0,       NULL,             NULL, NULL  },
   { d_slider_proc,     44,   264,  192,  24,   255,  8,    0,       0,          255,           255,     NULL,             NULL, NULL  },
   { d_slider_proc,     44,   292,  192,  24,   255,  8,    0,       0,          255,           127,     NULL,             NULL, NULL  },
   { d_yield_proc,      0,    0,    0,    0,    0,    0,    0,       0,          0,             0,       NULL,             NULL, NULL  },
   { NULL,              0,    0,    0,    0,    0,    0,    0,       0,          0,             0,       NULL,             NULL, NULL  }
};



int main(void)
{
   if (allegro_init() != 0)
      return 1;
   install_keyboard();
   install_mouse();
   install_timer();

   set_color_depth(16);

   if (set_gfx_mode(GFX_AUTODETECT, 240, 320, 0, 0) != 0) {
      set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
      allegro_message("Error setting graphics mode\n%s\n", allegro_error);
      return 1;
   }

   if (serial_midi_init()) {
      set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
      allegro_message("Error initializing serial port\n%s\n", ustrerror(errno));
      return 1;
   }

   set_dialog_color(thedialog, 0, makecol(224, 224, 224));
   do_dialog(thedialog, INSTLIST);

   return 0;
}

END_OF_MAIN()
