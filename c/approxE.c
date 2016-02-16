/******************************************************************************
 * Name:	Mustafa Abdullah
 * Date:	Feb. 28th 2012
 * Class:	ENPM808M
 * Brief:	The value of e can be expressed as an infinite series. Write a
 * 			program that approximates e by computing the value of the above
 * 			series until the current term becomes less than epsilon, where
 * 			epsilon is a small (floating point) number entered by the user.
 *
 * Homework 1, Problem 5
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	float eps, curVal, total = 0;
	int n = 0, nFact = 1;

	printf("Enter a small epsilon: ");
	scanf("%f", &eps);

	do
	{
		/* Case for n = 0 */
		if (!n)
			curVal = total = 1;
		else
		{
			nFact *= n;
			curVal = 1.0 / nFact;
			total += curVal;
		}
		++n;

	} while (curVal >= eps);

	printf("approximate e = %f\n", total);

	return 0;
}
