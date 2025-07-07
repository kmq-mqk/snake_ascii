#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <termios.h>

double GetCurrentTime() {
	struct timeval tv;

	gettimeofday(&tv, NULL);
	
	return (double)tv.tv_usec / 1e6 + tv.tv_sec;
}

void Render(int row, int col, int x, int y);

void Input(int* dx, int* dy);
void Wasd(char event[], int* dx, int* dy);
void Arrow(char event[], int* dx, int* dy);

void InGame();
void OffGame();

const double delta = 0.1;
struct termios originalTermios;

int main() {
	tcgetattr(STDIN_FILENO, &originalTermios);

	FILE* f = fopen("input.log", "w");
	fclose(f);


	int row, col;
	printf("Enter size of playground [row\tcolumn]:\t");
	scanf("%d %d", &row, &col);
	getchar();

	int x = col / 2;
	int y = row / 2;

	Render(row, col, x, y);

	int dx = 0, dy = 0;

	int running = 0;

	Input(&dx, &dy);
	x += dx;
	y += dy;

	if (dx != 0 || dy != 0)
		printf("received input!!!\n");

	running = 1;
	double time_last = GetCurrentTime();
	double time_current;

	printf("\033[?25l");  // hide cursor
	InGame();
	FILE* timeLog = fopen("timeDiff.log", "w");
	while (running++) {
		Input(&dx, &dy);

		time_current = GetCurrentTime();
		double timediff = time_current - time_last;

		if (timediff > 0.5 - delta && timediff <= 0.5 + delta) {
			fprintf(timeLog, "%d -> timeDiff == %lf\n", running, timediff);

			Render(row, col, x, y);

			x += dx;
			y += dy;

			time_last = time_current;
		}
	}

	return 0;
}

void Render(int row, int col, int x, int y) {
	printf("\033[H\033[0J");

	for (int j = 0; j < row; j++) {
		for (int i = 0; i < col; i++) {
			if (i == 0 || i == col - 1 || j == 0 || j == row - 1)
				printf("#");
			else if (i == x && j == y)
				printf("O");
			else
				printf("-");
		}
		printf("\r\n");

		fflush(stdout);
	}

	printf("\r\n------- PRESS <ESC> TO EXIT THE GAME -------\r\n");
}

void Input(int* dx, int* dy) {
	char event[4];

	size_t n = read(STDIN_FILENO, event, 4);
	if (n > 0 && n < sizeof(event))
		event[n] = 0;

	if (n == 0)
		return;

	if (n == 1) {
		if (event[0] == 27) {
			OffGame();
			printf("\n*********** [ GAME EXITED! ] ***********\n");
			printf("\033[?25h");	// show cursor
			exit(EXIT_SUCCESS);
		}
		else
			Wasd(event, dx, dy);
	}
	else
		Arrow(event, dx, dy);

//	if (event[0] == 27)
//		Arrow(event, dx, dy);
//	else
//		Wasd(event, dx, dy);

	FILE* fout = fopen("input.log", "a");
	fprintf(fout, "\t%s\n\n", event);
	fclose(fout);
}
void Arrow(char event[], int* dx, int* dy) {
	switch (event[2]) {
		case 'A': // up
			*dx = 0;
			*dy = -1;
			break;
		case 'B': // down
			*dx = 0;
			*dy = 1;
			break;
		case 'C': // right
			*dx = 1;
			*dy = 0;
			break;
		case 'D': // left
			*dx = -1;
			*dy = 0;
			break;
	}
}
void Wasd(char event[], int* dx, int* dy) {
	switch (event[0]) {
		case 'w': // up
			*dx = 0;
			*dy = -1;
			break;
		case 's': // down
			*dx = 0;
			*dy = 1;
			break;
		case 'd': // right
			*dx = 1;
			*dy = 0;
			break;
		case 'a': // left
			*dx = -1;
			*dy = 0;
			break;
	}
}

void OffGame() {
	tcsetattr(STDIN_FILENO, TCSANOW, &originalTermios);
}
void InGame() {
	struct termios inGameTermios = originalTermios;

	inGameTermios.c_lflag &= ~(ECHO | ICANON);
	// Case C:
	inGameTermios.c_cc[VMIN] = 0;
	inGameTermios.c_cc[VTIME] = 1;	// 0.1s

	tcsetattr(STDIN_FILENO, TCSANOW, &inGameTermios);
}
