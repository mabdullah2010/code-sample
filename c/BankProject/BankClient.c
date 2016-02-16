/******************************************************************************
 * ENPM808M - Final Project
 * Scott Robinson & Mustafa Abdullah
 * BankClient.c : Defines the client for the bank system.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <Time.h>
#include <conio.h>
#include "BankLib.h"

// Forward declarations
char parsePacket(char buffer[BUFFER_SIZE]);
void timedRead(char *buff );
void usage();
void DieWithUsage(char *errorMessage);
int recvDecrypt(int sd, char *buffer, int bufLen, int flags);

// Global Variables
char hideInput = FALSE;

int main(int argc, char* argv[])
{
	// Variables for socket communications
	WSADATA w;
    SOCKET sd;
	struct sockaddr_in server;
    SOCKADDR *s = (SOCKADDR *)&server;
    int sLen = (int)sizeof(struct sockaddr_in);
	char buffer[BUFFER_SIZE];
	int bytesReceived = 0;
	char done = FALSE;

	// Variables for user input IP triplets and port number
	unsigned short trip1, trip2, trip3, trip4;
	unsigned short portNum;

	// Check for correct number of command line arguments
	if (argc != 3)
        DieWithUsage("Incorrect number of command line arguments.");

	// Verify command line arguments
	if (sscanf(argv[1], "%u.%u.%u.%u", &trip1, &trip2, &trip3, &trip4) != 4)
		DieWithUsage("User input IP address could not be read.");
	
    if (trip1 > 255 || trip2 > 255 || trip3 > 255 || trip4 > 255)
		DieWithUsage("User input IP address is invalid.");

	if (sscanf(argv[2], "%u", &portNum) != 1)
		DieWithUsage("User input port number could not be read.");

	if (portNum < 256 || portNum > 65535)
		DieWithUsage("User input port number is invalid.");

	// Start Windows sockets
	if (WSAStartup(MAKEWORD(2, 0), &w))
		DieWithError("Could not open Windows socket connection.");

	// Open the socket
	sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);	// AF_INET,SOCK_DGRAM,IPPROTO_UDP defines UDP
    if (sd == INVALID_SOCKET)
        DieWithError("Could not create socket.");

	// Clear server struct
	memset((void *)&server, '\0', sizeof(struct sockaddr_in));

	// Set server parameters
	server.sin_family = AF_INET;
	server.sin_port = htons(portNum);

	// Set server IP address (argv[1]).
	server.sin_addr.s_addr = inet_addr(argv[1]);


    // Initiate the connection
    {
		char connect[] = "connect";
		char ipAddr[16];
		strcpy(ipAddr, argv[1]);
        if (sendEncrypt(sd, connect, BUFFER_SIZE, 0, s, sLen) == SOCKET_ERROR)
            DieWithError("sendto() failed. Check settings and retry.");
		else
			sendEncrypt(sd, ipAddr, BUFFER_SIZE, 0, s, sLen);
    }
   
    // Main loop which allows client to act as a terminal
    while(1)
    {
		if (done==TRUE)			// Server is waiting for client inputs
		{
			timedRead(buffer);  // Get user input
			if (sendEncrypt(sd, buffer, sizeof(buffer), 0, s, sLen) == SOCKET_ERROR)
				DieWithError("sendto() failed.");
			done = FALSE;		// Look for server message
        }
        else
		{
			recvDecrypt(sd, buffer, BUFFER_SIZE, 0);
			if (strcmp(buffer, "close")==0)
			{
				closesocket(sd);			// Close socket and exit
				WSACleanup();
				exit(0);
			}
			else
				done = parsePacket(buffer); // Act on the data recv from server
		}
    }
    //Does not reach here
}

char parsePacket(char buffer[BUFFER_SIZE])
{
    if (strcmp(buffer, "done")==0)  // Check if the server has indicated that it is finished
        return TRUE;				// return TRUE to indicate that the transmission is complete

    else if (buffer[0] != '\0') // Check if the buffer is empty
    {
		if (strstr(buffer, "password") || strstr(buffer, "Password")) // Check if the server is prompting for a password
			hideInput = TRUE;				 // Set hide input flag
		else
			hideInput = FALSE;

        printf("%s", buffer);   // Print out the Server message
        buffer[0] = '\0';       // Clean out the buffer
		return FALSE;           // return FALSE to indicate more data is expected
    }
}

void timedRead(char *buf)
{
    time_t start, now;
    char c = '\0';
	int i = 0;

    // Start the timer
    start = time(NULL);
    now = start;

    // Store all characters until return is hit or buffer is full
    while (c != '\r' && i != BUFFER_SIZE-1)
    {
        now = time(NULL);		// Set "now" to the current time
        if (kbhit())
        {
			// Reset timer
            start = time(NULL);
			
			// Avoids backspacing the prompts
			c = getch();
			if (c=='\t' || (c==0x08 && i==0))
				continue;

			// Display the input to user
			if (hideInput && c!=0x08 && c!='\r')			
				putchar('*');	// Display '*' instead of input chars
			else
				putchar(c);

			// Process the data
			if (c==0x08 && i>0) // Check for backspace
			{
				i--;			// Decrement input counter on backspace	
				putchar(0x00);	// Remove previous char and backspace
				putchar(0x08);
			}
			else if (c!=0x08)	// Store if valid input char
			{
				buf[i] = c;		// Store the input char
				i++;			// Increment the counter
			}
        }
        else if (difftime(now, start) >= 60)
        {
            printf("\n\nClient has been inactive for 60 seconds. Logging Out.\n");
			strcpy(buf, "timeout");	// Insert the logout command into the buffer
			return;					// Exit the function to avoid improper termination
        }
    }

    // Null terminate string at end of input to mask '\r'
	buf[i-1]= '\0';
}

// Usage message to be displayed when incorrect arguments are provided.
void usage()
{
    printf("   Usage: BankClient.exe <server_ip> <port>\n");
    printf("     server_ip - ip address of target server\n");
    printf("     port - port number of target server (256 - 65535)\n");
}

void DieWithUsage(char *errorMessage)
{
    printf("%s\n", errorMessage);
    usage();
    exit(0);
}

// Specific to client
int recvDecrypt(int sd, char *buffer, int bufLen, int flags)
{
	int i = recv(sd, buffer, bufLen, 0);
	Crypt(buffer, bufLen, 1);
	return i;
}
