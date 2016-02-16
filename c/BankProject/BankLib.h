/******************************************************************************
 * ENPM808M - Final Project
 * Scott Robinson & Mustafa Abdullah
 * BankLib : Defines commonalities between the client and server to prevent
 *           duplication.
 *****************************************************************************/

#include <stdio.h>
#include <WinSock2.h>

#define BUFFER_SIZE 128

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

// Display an error message and exit program.
void DieWithError(char *errorMessage)
{
	fprintf(stderr,"%s: %d\n", errorMessage, WSAGetLastError());
	WSACleanup();
	exit(1);
}

// Encrypt/Decrypt
void Crypt(char *buffer, int len, char dir)
{
    signed int xorKey = -1, addKey=1011;
    int i, j;

	for(j=0; buffer[j] != '\0'; j++){}

    if (dir == 0)
    {
        for (i=0; i<j; i++)
        {
            buffer[i] = (buffer[i] ^ xorKey) + addKey;
        }
    }
    else if (dir == 1)
    {
        for (i=0; i<j; i++)
        {
            buffer[i] = (buffer[i] - addKey) ^ xorKey;
        }
    }
}

int sendEncrypt(int sd, char *buffer, int bufLen, int flags, SOCKADDR *c, int cLen)
{
	int i;
	char sendBuf[BUFFER_SIZE];

	strcpy(sendBuf, buffer);

	Crypt(sendBuf, bufLen, 0);
	i = sendto(sd, sendBuf, bufLen, 0, c, cLen);
	return i;
}
