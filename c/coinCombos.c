/******************************************************************************
 * Name:	Mustafa Abdullah
 * Date:	Feb. 28th 2012
 * Class:	ENPM808M
 * Brief:	Write a program that asks user to enter a dollar amount X.
 * 			Display all combination of coins (quarter, dime, nickel, penny)
 * 			equal to X.
 *
 * Homework 1, Problem 2
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#define QUARTER 25
#define DIME 10
#define NICKEL 5
#define PENNY 1

int main(void)
{
	float nFloat;
	int amountOrig;
	int numQ, numD, numN, numP;

	printf("Enter a dollar amount: $");
	scanf("%f", &nFloat);

	/* Convert dollar amount to whole number to ease calculations */
	amountOrig = nFloat * 100;

	/*
	 * First calculate max number of quarters, then calculate all permutations
	 * of other coins. To do that, first calculate max number of dimes, then
	 * calculate all permutations of other coins... and so on. Then subtract 1
	 * from the number of quarters and calculate the permutations again.
	 */
	numQ = amountOrig / QUARTER;
	while (numQ >= 0)
	{
		int amountQ = amountOrig;

		amountQ -= QUARTER * numQ;

		numD = amountQ / DIME;
		while (numD >= 0)
		{
			int amountD = amountQ;

			amountD -= DIME * numD;

			numN = amountD / NICKEL;
			while (numN >= 0)
			{
				int amountN = amountD;
				amountN -= NICKEL * numN;

				numP = amountN;

				printf("%d quarter, %d dime, %d nickel, %d penny\n", numQ, numD, numN, numP);

				--numN;
			}
			--numD;
		}
		--numQ;
	}

	return 0;
}
