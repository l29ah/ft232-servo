/* This program is distributed under the GPL, version 2 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <ftdi.h>
#include <pthread.h>
#include <termios.h>

struct ftdi_context *ftdi;
bool enabled = true;

static void ftdi_write_data_(struct ftdi_context *ftdi, unsigned char *buf, int size) {
	if (ftdi_write_data(ftdi, buf, 1) < 0)
		exit(1);
}

static void move_to(uint_fast8_t pos)
{
	uint8_t buf[1];
	uint_fast16_t duty = 1000 + 1000 * pos / 180;
	//uint_fast16_t duty = 1000+1000 * pos / 90;
	if (enabled) {
		*buf = 0xff;
		ftdi_write_data_(ftdi, buf, 1);
	}
	usleep(duty);
	*buf = 0;
	ftdi_write_data_(ftdi, buf, 1);
	usleep(20000 - duty);
}
void *servo_thread(void *pos_)
{
	uint8_t *pos = pos_;
	while (1) {
		uint_fast8_t cur_pos = *pos;
		move_to(cur_pos);
	}
	return NULL;
}

int main(int argc, char **argv)
{
	int f,i;
	unsigned char buf[1];
	int retval = 0;

	if ((ftdi = ftdi_new()) == 0)
	{
		fprintf(stderr, "ftdi_new failed\n");
		return EXIT_FAILURE;
	}

	//f = ftdi_usb_open(ftdi, 0x0403, 0x6010);
	f = ftdi_usb_open(ftdi, 0x0403, 0x6001);

	if (f < 0 && f != -5)
	{
		fprintf(stderr, "unable to open ftdi device: %d (%s)\n", f, ftdi_get_error_string(ftdi));
		retval = 1;
		goto done;
	}

	/* set the terminal to raw mode */
	struct termios orig_term_attr;
	struct termios new_term_attr;
	tcgetattr(fileno(stdin), &orig_term_attr);
	memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
	new_term_attr.c_lflag &= ~(ECHO|ICANON);
	tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

	ftdi_set_bitmode(ftdi, 8, BITMODE_BITBANG);
	uint8_t value = 0;
	pthread_t thread;
	pthread_create(&thread, NULL, servo_thread, &value);
	while (1) {
		int input = fgetc(stdin);
		value++;
		printf("%d\n", value);
		if (input == EOF) break;
		/*
		value = 0;
		sleep(1);
		value = 255;
		sleep(1);
		*/
		/*
		move_to(0);
		sleep(1);
		move_to(255);
		sleep(1);
		*/
	}
	ftdi_disable_bitbang(ftdi);

	ftdi_usb_close(ftdi);
done:
	ftdi_free(ftdi);

	return retval;
}
