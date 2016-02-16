/******************************************************************************
 * Name:	Mustafa Abdullah
 * Date:	Feb. 28th 2012
 * Class:	ENPM808M
 * Brief:	Write a program that asks the user for a two-digit number, then
 * 			prints the English word for the number (don’t forget the numbers
 * 			that need special treatment).
 *
 * Homework 1, Problem 4
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
	char *oneToNineteenText[] = {"", "one", "two", "three", "four", "five", "six",
		"seven", "eight", "nine", "ten", "eleven", "twelve", "thirteen",
		"fourteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen"};

	char *tensToText[] = {"", "", "twenty", "thirty", "forty", "fifty", "sixty",
		"seventy", "eighty", "ninety"};
	int number;
	char numberStr[32];

	while(1)
	{
		printf("Enter a two-digit number: ");

		/* Needed in case user inputs something dumb. */
		if (!scanf("%d", &number))
			while (getchar() != '\n');

		/* Validate it's a two digit number. */
		if (number < 10 || number > 99)
		{
			printf("Please enter a valid number.\n");
			continue;
		}

		/* Handle the teens separately. */
		if (number >= 10 && number < 20)
			strcpy (numberStr, oneToNineteenText[number]);

		/* If number is divisible by 10, only needs value from one array */
		else if (!(number % 10))
			strcpy (numberStr, tensToText[number / 10]);
		else
			sprintf (numberStr, "%s-%s", tensToText[number / 10],
				oneToNineteenText[number % 10]);

		break;
	}

	printf("You entered the number %s.\n", numberStr);
	return 0;
}
