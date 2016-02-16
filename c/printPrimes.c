/******************************************************************************
 * Name:	Mustafa Abdullah
 * Date:	Feb. 28th 2012
 * Class:	ENPM808M
 * Brief:	Write a program that prompts user to enter a number N.
 * 			a. Print all prime numbers that are less than or equal to N.
 *			b. Display the number of prime numbers that your program has found.
 *
 * Homework 1, Problem 1
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	int n, numPrimes = 0;

	printf("Enter a number: ");
	scanf("%d", &n);

	/* Loop from lowest possible prime (2) up to number entered. */
	/* TODO: Figure out how to implement prime factorization to optimize code. */
	for (int i = 2; i <= n; ++i)
	{
		int j = 2;
		for (; i % j; ++j)
			/* Empty body. */;

		/* If j is able to reach i, then nothing is divisible by i other than 1 and
		 * itself, therefore i is prime.
		 */
		if (j == i)
		{
			printf("%d ", i);
			++numPrimes;
		}
	}

	printf("\nTotal primes: %d\n", numPrimes);


	return 0;
}
