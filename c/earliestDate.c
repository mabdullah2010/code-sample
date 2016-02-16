/******************************************************************************
 * Name:	Mustafa Abdullah
 * Date:	Feb. 28th 2012
 * Class:	ENPM808M
 * Brief:	Write a program that prompts the user to enter a series of dates
 * 			and then indicate which date comes earlier on the calendar. The
 * 			user will enter 0/0/0 to indicate that no more dates will be
 * 			entered.
 *
 * Homework 1, Problem 3
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

int dateIsValid(unsigned short m, unsigned short d, unsigned short y)
{
	short monthLengths[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	if (y > 0 &&						// validate year
		m > 0 && m < 13 &&				// validate month
		d > 0 && d <= monthLengths[m])	// validate day
	{
		return 1;
	}

	return 0;
}

/* Returns 1 if date1 is earlier, returns 0 otherwise. */
int findEarlierDate(unsigned short m1, unsigned short d1, unsigned short y1,
					unsigned short m2, unsigned short d2, unsigned short y2)
{
	if (y1 < y2)			// Compare year
		return 1;
	else if (y2 < y1)
		return 0;
	else
	{
		if (m1 < m2)		// Compare month
			return 1;
		else if (m2 < m1)
			return 0;
		else
		{
			if (d1 < d2)	// Compare day
				return 1;
			else
				return 0;
		}
	}
}

int main(void)
{
	char date[16];
	unsigned int m, d, y;
	unsigned int mEarly = 0, dEarly, yEarly;
	while (1)
	{
		printf("Enter a date mm/dd/yyyy: ");
		scanf("%s", date);

		/* If the format doesn't match, display usage message. */
		if (sscanf(date, "%u/%u/%u", &m, &d, &y) != 3)
		{
			printf("Please use valid date format (mm/dd/yyyy)\n");
			continue;
		}

		/* If all numbers are zero, break the loop. */
		if (!(m || d || y))
		{
			break;
		}

		if (!dateIsValid(m, d, y))
		{
			printf("Date out of range.\n");
			continue;
		}

		/* Initial case */
		if (mEarly == 0)
		{
			mEarly = m;
			dEarly = d;
			yEarly = y;
		}
		else
		{
			/* If new date entered is earlier than earliest seen date so far,
			 * update earliest date.
			 */
			if (findEarlierDate(m, d, y, mEarly, dEarly, yEarly))
			{
				mEarly = m;
				dEarly = d;
				yEarly = y;
			}
		}
	}

	printf("%02u/%02u/%04u is the earliest date.\n", mEarly, dEarly, yEarly);

	return 0;
}


