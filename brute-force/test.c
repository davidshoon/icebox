#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	char *test_string = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

	int i, j, k;


	// nested for loops... (cons: cause you need to make as many nested loops as the number of output chars)
	// (pros: it's easy to make and understand, and all you have to do is copy-paste for-loops. This is good for small number of output chars, which is what
	// you normally brute force anyway)

/*
	for (i = 0; i < strlen(test_string); i++) {
		for (j = 0; j < strlen(test_string); j++) {
			for (k = 0; k < strlen(test_string); k++) {
				printf("%c%c%c\n", test_string[i], test_string[j], test_string[k]);
			}
		}
	}
*/

	// another way of doing it..

	char index[3] = {0, 0, 0};

	// index incrementor, using while loop with incrementing the index and checking for it using if-conditions
	// (bad: cause you need as many if-conditions as the number of output chars)

/*

	while (index[2] != strlen(test_string)) {
		printf("%c%c%c\n", test_string[index[2]], test_string[index[1]], test_string[index[0]]);

		index[0]++;
		if (index[0] == strlen(test_string)) {
			index[0] = 0;
			index[1]++;
		}

		if (index[1] == strlen(test_string)) {
			index[1] = 0;
			index[2]++;
		}
	}

*/

	// improving upon the above...
	// another way of doing it... arbitrary number of (total)... just change the printf() to a for loop and you're set... 

	while (1) {

		// NB: you can change this printf to use a for loop to output as many chars as you want.
		printf("%c%c%c\n", test_string[index[2]], test_string[index[1]], test_string[index[0]]);

		for (i = 0; i < sizeof(index); i++) {
			index[i]++;

			// index[2] just happens to be the "last" index required to be updated. Change this as appropriate.
			if (index[2] == strlen(test_string)) {
				goto end_loop;
			}
			else if (index[i] == strlen(test_string)) {
				index[i] = 0;
			}
			else {
				break;
			}
		}
	}

end_loop:

	return 0;
}
