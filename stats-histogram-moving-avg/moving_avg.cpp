#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <algorithm>

void printboth(int moving_avg, int current)
{
	int spaces_moving_avg = (double) moving_avg / 255.0 * 80.0;
	int spaces_current = (double) current / 255.0 * 80.0;

	int spaces_min = std::min(spaces_current, spaces_moving_avg);
	int spaces_max = std::max(spaces_current, spaces_moving_avg);

	bool moving_avg_first = false;


	for (int i = 0; i < spaces_min; i++) {
		printf(" ");
	}

	if (spaces_current < spaces_moving_avg) {
		printf("-");
	}
	else {
		moving_avg_first = true;
		printf("*");
	}

	for (int i = 0; i < spaces_max - spaces_min; i++) {
		printf(" ");
	}

	if (moving_avg_first) {
		printf("-\n");
	}
	else {
		printf("*\n");
	}

}

void printstar(int x)
{
	int spaces = (double) x / 255.0 * 80.0;
	for (int i = 0; i < spaces; i++) {
		printf(" ");
	}

	printf("*\n");
}

int main()
{
	int c;
	int moving_avg = 0;
	int average_total = 0;
	int count = 0;

	while ((c = getc(stdin)) != EOF) {
// This performs an exponential weighted moving average, or a first-order IIR filter.
		moving_avg = moving_avg * 0.9 + c * 0.1;
		average_total += c;
		count++;

//		printf("%d\n", moving_avg);
//		printstar(moving_avg);
		printboth(moving_avg, c);
	}

	printf("actual average: %d\n", average_total / count);

}
