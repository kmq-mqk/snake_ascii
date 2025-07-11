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

void Render(int col, int row, int x, int y);
void StatusCheck(int col, int row, int x, int y, int* running);
void GameOver(int* running);

void Input(int* dx, int* dy);
void Wasd(char event[], int* dx, int* dy);
void Arrow(char event[], int* dx, int* dy);

void InGame();
void OffGame();

const double delta = 0.1;
struct termios originalTermios;

int main() {
	while (1) {
		printf("\033[H\033[J\033[3J");
	
		tcgetattr(STDIN_FILENO, &originalTermios);
	
		FILE* f = fopen("input.log", "w");
		fclose(f);
	
	
		int row, col;
		printf("Enter size of playground [row\tcolumn]:\t");
		scanf("%d %d", &row, &col);
	
		int x = col / 2;
		int y = row / 2;
	
		Render(col, row, x, y);
	
		int dx = 0, dy = 0;
	
		int running = 0;
	
		InGame();
		while (1) {
			Input(&dx, &dy);
	
			if (dx != 0 || dy != 0) {
				printf("received input!!!\n");
				x += dx;
				y += dy;
				running = 1;
	
				break;
			}
		}
	
		double time_last = GetCurrentTime();
		double time_current;
	
		FILE* timeLog = fopen("timeDiff.log", "w");
		while (running++) {
			Input(&dx, &dy);
	
			time_current = GetCurrentTime();
			double timediff = time_current - time_last;
	
			if (timediff > 0.5 - delta && timediff <= 0.5 + delta) {
				fprintf(timeLog, "%d -> timeDiff == %lf\n", running, timediff);
	
				Render(col, row, x, y);
	
				x += dx;
				y += dy;
	
				StatusCheck(col, row, x, y, &running);
	
				time_last = time_current;
			}
		}
	
		OffGame();
	}

	return 0;
}

void Render(int col, int row, int x, int y) {
	printf("\033[H\033[J\033[3J");
	fflush(stdout);

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
	}

	printf("\r\n------- PRESS <ESC> TO EXIT THE GAME -------\r\n");
}

void Input(int* dx, int* dy) {
	FILE* fout = fopen("input.log", "a");
	char event[5] = {};

	fflush(stdout);

	struct termios curTermios;
	tcgetattr(STDIN_FILENO, &curTermios);
	int canon = (curTermios.c_lflag & ICANON) != 0;
	if (canon)
		fprintf(fout, "CANON -> %d\r\n", canon);
	else
		fprintf(fout, "NOT CANON -> %d\r\n", canon);

	size_t n = read(STDIN_FILENO, event, sizeof(event));
	if (n > 0 + canon && n < sizeof(event))
		event[n] = 0;

	if (n == 0 + canon)
		return;

	if (n == 1 + canon) {
		if (event[0] == 27) {
			printf("\n*********** [ GAME EXITED! ] ***********\n");
			OffGame();
			exit(EXIT_SUCCESS);
		}
		else
			Wasd(event, dx, dy);
	}
	else
		Arrow(event, dx, dy);

	fprintf(fout, "\t%s\r\n\n", event);
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
	printf("\033[?25h");	// show cursor
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTermios);
}
void InGame() {
	printf("\033[?25l");  // hide cursor
	
	struct termios inGameTermios = originalTermios;

	inGameTermios.c_lflag &= ~(ECHO | ICANON);
	// Case C:
	inGameTermios.c_cc[VMIN] = 0;
	inGameTermios.c_cc[VTIME] = 1;	// 0.1s

	tcsetattr(STDIN_FILENO, TCSANOW, &inGameTermios);
}


void StatusCheck(int col, int row, int x, int y, int* running) {
	if ((x < 0 || y < 0) || (x >= col || y >= row))
		GameOver(running);
}
void GameOver(int* running) {
	printf("\033[H\033[J\033[3J");
	fflush(stdout);
//	OffGame();

	printf("\n\t[[[\tWELL, YOU DEAD!!\t]]]\n");

	printf("\r\n------- PRESS <SPACE> TO REPLAY -------\r\n");
	printf("\r\n------- PRESS <ESC> TO EXIT THE GAME -------\r\n");

	struct termios curTermios;
	tcgetattr(STDIN_FILENO, &curTermios);
	int canon = (curTermios.c_lflag & ICANON) != 0;

	char input[1 + canon] = {};
	size_t n = 0;
	while (1) {
		n = read(STDIN_FILENO, input, 1 + canon);
		if (n > 0 && (input[0] == 27 || input[0] == 32))
			break;
	}

	switch (*input) {
		case 27:
			OffGame();
			printf("\n*********** [ GAME EXITED! ] ***********\n");
			exit(EXIT_SUCCESS);
			break;
		case 32:
			*running = -1;
			break;
	}
}
