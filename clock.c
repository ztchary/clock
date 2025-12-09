#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <string.h>
#include <signal.h>

struct termios original_termios;

#define FONT_WIDTH 10
#define FONT_HEIGHT 8

char *fontdata =
"  /######     /##      /######   /###### /##   /## /#######   /######  /########  /######   /######           "
" /###_  ##  /####    /##__  ## /##__  ##| ##  | ##| ##____/  /##__  ##|_____ ##/ /##__  ## /##__  ##          "
"| ####? ## |_  ##   |__/  ? ##|__/  ? ##| ##  | ##| ##      | ##  ?__/     /##/ | ##  ? ##| ##  ? ##    /##   "
"| ## ## ##   | ##     /######/   /#####/| ########| ####### | #######     /##/  |  ######/|  #######   |__/   "
"| ##? ####   | ##    /##____/   |___  ##|_____  ##|_____  ##| ##__  ##   /##/    >##__  ## ?____  ##          "
"| ## ? ###   | ##   | ##       /##  ? ##      | ## /##  ? ##| ##  ? ##  /##/    | ##  ? ## /##  ? ##    /##   "
"|  ######/  /###### | ########|  ######/      | ##|  ######/|  ######/ /##/     |  ######/|  ######/   |__/   "
" ?______/  |______/ |________/ ?______/       |__/ ?______/  ?______/ |__/       ?______/  ?______/           ";

//#define FONT_WIDTH 8
//#define FONT_HEIGHT 7

//char *fontdata =
//" ######    ##     ######  ########    #########  ###### ######## ######  ######         "
//"###   ## ####   ##    ####    ####    ####      ##    ##     ## ##    ####    ##   ##   "
//"####  ##   ##         ##      ####    ####      ##          ##  ##    ####    ##   ##   "
//"## ## ##   ##    ######   ##### ############### #######    ##    ######  #######        "
//"##  ####   ##   ##            ##      ##      ####    ##  ##    ##    ##      ##   ##   "
//"##   ###   ##   ##      ##    ##      ####    ####    ## ##     ##    ####    ##   ##   "
//" ######  ###### ######## ######       ## ######  ###### ##       ######  ######         ";

void writedigit(int index) {
	for (int i = 0; i < FONT_HEIGHT; i++) {
		fwrite(fontdata + index * FONT_WIDTH + i * (FONT_WIDTH * 11), 1, FONT_WIDTH, stdout);
		printf("\e[1B\e[%dD", FONT_WIDTH);
	}
	printf("\e[%dC\e[%dA", FONT_WIDTH + 1, FONT_HEIGHT);
}

void get_wsize(int *row, int *col) {
	struct winsize ws;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
	*row = ws.ws_row;
	*col = ws.ws_col;
}

void enter() {
	tcgetattr(STDIN_FILENO, &original_termios);

	struct termios tios = original_termios;
	tios.c_lflag &= ~(ECHO);
	tios.c_iflag &= ~(ICRNL | INPCK | ISTRIP | IXON);
	tios.c_cflag |= (CS8);

	tios.c_cc[VMIN] = 0;
	tios.c_cc[VTIME] = 1;

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &tios);
	printf("\e[?1049h\e[?25l\e[H\e[J");
	fflush(stdout);
}

void leave(int sg) {
	(void)sg;
	printf("\e[?1049l\e[?25h");
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
	fflush(stdout);
	exit(0);
}

void winch(int sg) {
	(void)sg;
	printf("\e[H\e[J");
	fflush(stdout);
}

int main() {
	fontdata = strdup(fontdata);
	int rows, cols;
	for (int i = 0; i < strlen(fontdata); i++) {
		if (fontdata[i] == '?') fontdata[i] = '\\';
	}

	enter();
	signal(SIGINT, leave);
	signal(SIGWINCH, winch);

	for (;;) {
		get_wsize(&rows, &cols);
		time_t rawtime;
		struct timespec ts;
		struct tm *info;

		time(&rawtime);
		clock_gettime(CLOCK_REALTIME, &ts);

		info = localtime(&rawtime);

		int hour = (info->tm_hour + 11) % 12 + 1;
		int minute = info->tm_min;
		int second = info->tm_sec;
		int msec = ts.tv_nsec / 1000000;

		printf("\e[%d;%dH", rows / 2 - FONT_HEIGHT / 2 + 1, cols / 2 - FONT_WIDTH * 4 - 3);
		writedigit(hour/10);
		writedigit(hour%10);
		if (msec > 500) printf("\e[37m");
		writedigit(10);
		printf("\e[m");
		writedigit(minute/10);
		writedigit(minute%10);
		if (msec > 500) printf("\e[37m");
		writedigit(10);
		printf("\e[m");
		writedigit(second/10);
		writedigit(second%10);
		fflush(stdout);
		usleep(500000);
	}
}
