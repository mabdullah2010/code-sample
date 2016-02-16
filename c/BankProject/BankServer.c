/******************************************************************************
 * ENPM808M - Final Project
 * Scott Robinson & Mustafa Abdullah
 * BankServer.c : Defines the server for the bank system.
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <WinSock2.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "BankLib.h"

#define RECV_DECRYPT(a, b, c, d, e) if(recvDecrypt(a, b, c, d, e) == (BUFFER_SIZE+1)) return -1;
#define BORDER "\n-----------------------------------------------------------------------\n"
#define MAX_NAME_LEN 16
#define MAX_IP_LEN 16
#define ACCT_NUM_LEN 6
#define TRUE 1
#define FALSE 0

// Defines a bank system user
struct User
{
    char firstName[MAX_NAME_LEN];
    char lastName[MAX_NAME_LEN];
    char username[MAX_NAME_LEN];
    char password[MAX_NAME_LEN];
    unsigned int acctNum;
    char userType;
    unsigned int balance;
    char ipAddr[MAX_IP_LEN];
    time_t lastLogin;
    struct User *next;
};
typedef struct User User;

static User ZERO_USER;  // used to initialize structs

// Forward declarations
void sendDone(int sd, SOCKADDR *c, int cLen);
User *login(int sd, SOCKADDR *c, int cLen, User *head, char *ipAddr);
User *readDatabase(const char *dbPath);
void writeDatabase(const char *dbPath, User *head);
User *findUserByUsername(User *head, const char *username);
User *findUserByAcctNum(User *head, int acctNum);
void freeUsers(User *head);
void usage();

char ynOption(char *message, int sd, SOCKADDR *c, int cLen);	// helper function
int seeBalanceSelected(int sd, SOCKADDR *c, int cLen, User *head);
void seeUserBalance(int sd, SOCKADDR *c, int cLen, User *user);
int transferBetweenAccounts(int sd, SOCKADDR *c, int cLen, User *head);
int transferToCustomer(int sd, SOCKADDR *c, int cLen, User *user, User *head);
int addNewCustomer(int sd, SOCKADDR *c, int cLen, User **head);
int changePassword(int sd, SOCKADDR *c, int cLen, User *user);
void totalBalanceOfAll(int sd, SOCKADDR *c, int cLen, User *head);
void listCustomer(int sd, SOCKADDR *c, int cLen, User *user);
void listCustomers(int sd, SOCKADDR *c, int cLen, User *head);
void sortByBalance(int sd, SOCKADDR *c, int cLen, User **head);
int recvDecrypt(int sd, char *buffer, int bufLen, SOCKADDR *c, int *cLen);

int main(int argc, char *argv[])
{
    static struct sockaddr_in ZERO_SOCKADDR;

    WSADATA w;
    struct sockaddr_in client, server = ZERO_SOCKADDR;
    SOCKADDR *c = (SOCKADDR *)&client;
    int cLen = (int)sizeof(struct sockaddr_in);
    int sd = 0;
    char *dbPath = NULL;
    unsigned short port = 0;
    User *head = NULL;
	char doLogin = FALSE;
	char doShutDown = FALSE;
	time_t currTime;
	int option = 0;
	char ipAddr[MAX_IP_LEN];
	char initialized = FALSE;

    // Test for correct number of arguments
    if (argc != 3)
    {
       usage();
       exit(0);
    }

    dbPath = argv[1];
    port = atoi(argv[2]);

    // Validate port number
    if (!port || port < 256 || port > 65535)
    {
        fprintf(stderr, "Invalid port number.\n");
        usage();
        exit(0);
    }
    
    // Open and parse the database file
    head = readDatabase(dbPath);
    if (!head)
        DieWithError("Error reading database file.\n");

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 0), &w))
        DieWithError("WSAStartup() failed.");

    // Open a datagram socket
    sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sd == INVALID_SOCKET)
        DieWithError("Could not create socket.\n");

    // Clear out server struct
    server = ZERO_SOCKADDR;

    // Set family, port, and address
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind to the local address
    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
    {
        closesocket(sd);
        DieWithError("bind() failed");
    }

    printf("\nWaiting for connection...\n\n");

    for (;;) // Run forever
    {
		char buffer[BUFFER_SIZE];
		User *clientUser;

		// Shut down the server if the flag is set
		if (doShutDown == TRUE)
			break;

		// Wait for client to send initial packet to confirm connection
		if (!doLogin && recvDecrypt(sd, buffer, BUFFER_SIZE, c, &cLen) == SOCKET_ERROR)
		{
			fprintf(stderr, "recvfrom() failed.\n");
			continue;
		}

		// Login on initial connection and if flag is set
		if(!strcmp(buffer, "connect") || doLogin)
		{
			// Get IP address if first transmission
			if (!initialized)
			{	
				recvDecrypt(sd, buffer, BUFFER_SIZE, c, &cLen);
				sscanf(buffer, "%s", &ipAddr);
			}

			// Loop until successful login
			while (!(clientUser = login(sd, c, cLen, head, ipAddr)))
				sendEncrypt(sd, "\nInvalid credentials. Please try again.\n", BUFFER_SIZE, 0, c, cLen);
			initialized = TRUE;
		}

		// Display successful login
		currTime = time(NULL);
        printf("%.24s : %s %s has logged in from %s.\n", ctime(&currTime), clientUser->firstName,
            clientUser->lastName, clientUser->ipAddr);

        sprintf(buffer, "\n\nLogin successful. Welcome, %s %s!\n",
            clientUser->firstName, clientUser->lastName);
        sendEncrypt(sd, buffer, BUFFER_SIZE, 0, c, cLen);

        sprintf(buffer, "User type: %s\n", clientUser->userType == 'C' ? "Customer" : "Manager");
        sendEncrypt(sd, buffer, BUFFER_SIZE, 0, c, cLen);

        if ((long long)clientUser->lastLogin > 0)
		    sprintf(buffer, "Last login: %.24s\n", ctime(&clientUser->lastLogin)); 
	    else
		    sprintf(buffer, "Last login: N/A\n");
	    sendEncrypt(sd, buffer, BUFFER_SIZE, 0, c, cLen);

		sprintf(buffer, "Last IP address: %s\n\n", clientUser->ipAddr);
        sendEncrypt(sd, buffer, BUFFER_SIZE, 0, c, cLen);

        // Set the last login time to the current time
        clientUser->lastLogin = currTime;

        // Initialize login flag
		doLogin = FALSE;

        while (doLogin == FALSE && doShutDown == FALSE)
        {
            // Print menu for correct user type
            switch(clientUser->userType) {

            case 'M': // Manager
                sendEncrypt(sd, "\nRequest Code\t Action\n\n", BUFFER_SIZE, 0, c, cLen);
                sendEncrypt(sd, "1\t\t Report balance of a selected customer\n", BUFFER_SIZE, 0, c, cLen);
                sendEncrypt(sd, "2\t\t Transfer money between two accounts\n", BUFFER_SIZE, 0, c, cLen);
                sendEncrypt(sd, "3\t\t Add a new customer to the system\n", BUFFER_SIZE, 0, c, cLen);
                sendEncrypt(sd, "4\t\t See the list of customers and their account information\n", BUFFER_SIZE, 0, c, cLen);
                sendEncrypt(sd, "5\t\t Report customer accounts sorted by balance\n", BUFFER_SIZE, 0, c, cLen);
                sendEncrypt(sd, "6\t\t See the total balance of all customers\n", BUFFER_SIZE, 0, c, cLen);
                sendEncrypt(sd, "7\t\t Shut down the server\n", BUFFER_SIZE, 0, c, cLen);
                sendEncrypt(sd, "8\t\t Logout\n", BUFFER_SIZE, 0, c, cLen);
                break;

            case 'C': // Customer
                sendEncrypt(sd, "Request Code\t Action\n", BUFFER_SIZE, 0, c, cLen);
                sendEncrypt(sd, "1\t\t Report balance of the customer\n", BUFFER_SIZE, 0, c, cLen);
                sendEncrypt(sd, "2\t\t Transfer money to another customer account\n", BUFFER_SIZE, 0, c, cLen);
                sendEncrypt(sd, "3\t\t Change password\n", BUFFER_SIZE, 0, c, cLen);
                sendEncrypt(sd, "4\t\t Logout\n", BUFFER_SIZE, 0, c, cLen);
                break;

            default:    // Shouldn't reach here since it loops until successful login.
                break;
            }

            sendEncrypt(sd, "\nPlease enter a Request Code: ", BUFFER_SIZE, 0, c, cLen);
            sendDone(sd, c, cLen);

            // Blocking wait for menu option from client
            if(recvDecrypt(sd, buffer, BUFFER_SIZE, c, &cLen) == (BUFFER_SIZE+1)) { doLogin = TRUE; continue; }

			// Get user input
			if (sscanf(buffer, "%d", &option) != 1)
				option = -1;

            // Perform appropriate action
            if (clientUser->userType == 'M')  // Manager
            {
                switch(option) {
                    case 1: if(seeBalanceSelected(sd, c, cLen, head) == -1){ doLogin = TRUE;} break;
                    case 2: if(transferBetweenAccounts(sd, c, cLen, head) == -1){ doLogin = TRUE;} break;
                    case 3: if(addNewCustomer(sd, c, cLen, &head) == -1){ doLogin = TRUE;} break;
                    case 4: listCustomers(sd, c, cLen, head); break;
                    case 5: sortByBalance(sd, c, cLen, &head); listCustomers(sd, c, cLen, head); break;
                    case 6: totalBalanceOfAll(sd, c, cLen, head); break;
                    case 7: doShutDown = TRUE; break;	// Sets flag to shut down server
                    case 8: doLogin = TRUE; break;		// Sets flag to loop back to login state
                    default: sendEncrypt(sd, "\n\nInvalid input. Please enter a valid option.\n\n", BUFFER_SIZE, 0, c, cLen); break;
                }
            }
            else if (clientUser->userType == 'C') // Customer
            {
                switch(option) {
                    case 1: seeUserBalance(sd, c, cLen, clientUser); break;
                    case 2: if(transferToCustomer(sd, c, cLen, clientUser, head) == -1){ doLogin = TRUE;} break;
                    case 3: if(changePassword(sd, c, cLen, clientUser) == -1){ doLogin = TRUE;} break;
                    case 4: doLogin = TRUE; break; // Sets flag to loop back to login state
                    default: sendEncrypt(sd, "\n\nInvalid input. Please enter a valid option.\n\n", BUFFER_SIZE, 0, c, cLen); break;
                }
            }
            else{}
        }
		// Reached when log out OR shut down flag is set to true
		sendEncrypt(sd, "\n\nLogging out.", BUFFER_SIZE, 0, c, cLen);
		currTime=time(NULL);
		printf("%.24s : %s %s has logged out.\n\n", ctime(&currTime), clientUser->firstName, clientUser->lastName);
		writeDatabase(dbPath, head);

		if (doShutDown == FALSE)
			sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
    }
    // Reached when shut down flag is set to true
	sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
	sendEncrypt(sd, "\nThe server has shut down. Client is closing. Thank you.\n\n", BUFFER_SIZE, 0, c, cLen);
	sendEncrypt(sd, "close", BUFFER_SIZE, 0, c, cLen);	// message to close the client
	printf("Server is shutting down by client request.\n\n");
}

void sendDone(int sd, SOCKADDR *c, int cLen)
{
    sendEncrypt(sd, "done", BUFFER_SIZE, 0, c, cLen);
}

User *login(int sd, SOCKADDR *c, int cLen, User *head, char *ipAddr)
{
    char username[MAX_NAME_LEN];
    char password[MAX_NAME_LEN];
    User *user;
	int i;

    // Prompt for user name
    sendEncrypt(sd, "\nUsername: ", BUFFER_SIZE, 0, c, cLen);
    sendDone(sd, c, cLen);
    i = recvDecrypt(sd, username, MAX_NAME_LEN, c, &cLen);

    // Prompt for password
    sendEncrypt(sd, "\nPassword: ", BUFFER_SIZE, 0, c, cLen);
    sendDone(sd, c, cLen);
    recvDecrypt(sd, password, MAX_NAME_LEN, c, &cLen);

    // Search for the given username
    user = findUserByUsername(head, username);

	//Crypt(password, MAX_NAME_LEN, 0);

    // If found and password matches, return user type
    if (user && !strcmp(user->password, password))
	{
		// Get IP address
		strcpy(user->ipAddr, ipAddr);
        return user;
	}

    return NULL;
}

// Read the database file into the Users linked list
User *readDatabase(const char *dbPath)
{
    struct stat st;
    int filesize;
    int numUsers;
    User *head = NULL;
    
    // Grab the filesize to figure out how many users are stored
    stat(dbPath, &st);
    filesize = st.st_size;

    numUsers = filesize / sizeof(User);

    if (numUsers)
    {
        FILE *dbFile = NULL;
        int i = 0;
        User *curr, *prev = NULL;

        // Open the database file
        dbFile = fopen(dbPath, "r");
        if (!dbFile)
            return NULL;

        // Create first one outside of loop to set head
        curr = (User *)malloc(sizeof(User));
        if (!curr)
            return NULL;

        fread(curr, sizeof(User), 1, dbFile);
        curr->next = NULL;
        Crypt(curr->password, strlen(curr->password), 1);
        head = prev = curr;
        ++i;

        // Create the rest of the users
        for (; i < numUsers; prev = curr, ++i)
        {
            curr = (User *)malloc(sizeof(User));
            if (!curr)
            {
                freeUsers(head);
                return NULL;
            }

            fread(curr, sizeof(User), 1, dbFile);
            Crypt(curr->password, strlen(curr->password), 1);
            prev->next = curr;
            curr->next = NULL;
        }
    
        fclose(dbFile);
    }

    return head;
}

// Write the Users linked list into the database file
void writeDatabase(const char *dbPath, User *head)
{
    FILE *dbFile;
	User *tmp = head;
    
    // Open the database file
    dbFile = fopen(dbPath, "w");
    if (!dbFile)
    {
        fprintf(stderr, "Error opening database file for writing.");
        return;
    }

    while(head)
    {
        Crypt(head->password, strlen(head->password), 0);
        fwrite(head, sizeof(User), 1, dbFile);
        head = head->next;
    }
	while(tmp)
    {
        Crypt(tmp->password, strlen(tmp->password), 1);
        tmp = tmp->next;
    }

    fclose(dbFile);
}

// Search for a user with the passed in username. Returns NULL if user not found.
User *findUserByUsername(User *head, const char *username)
{
    while(head)
    {
        if (!strcmp(head->username, username)) // if they match
            return head;

        head = head->next;
    }

    return NULL;
}

// Search for a user with the passed in account number. Returns NULL if user not found.
User *findUserByAcctNum(User *head, int acctNum)
{
    while(head)
    {
        if (head->acctNum == acctNum) // if they match
            return head;

        head = head->next;
    }

    return NULL;
}

// Usage message to be displayed when incorrect arguments are provided.
void usage()
{
    printf("   Usage: BankServer.exe <database_file> <port>\n");
    printf("     database_file - path to database file\n");
    printf("     port - port number to listen for clients (256 - 65535)\n");
}

/*************************************************************************************************/

char ynOption(char *message, int sd, SOCKADDR *c, int cLen)
{
    char buffer[BUFFER_SIZE];

    // Find length of message string
    //for (int i=0; message[i]!='\0'; i++) {}

    // Send message and Y/N option
    sendEncrypt(sd, message, BUFFER_SIZE, 0, c, cLen);
    sendEncrypt(sd, "Y/N? ", BUFFER_SIZE, 0, c, cLen);
    sendDone(sd, c, cLen);

    // Receive user response
    recvDecrypt(sd, buffer, BUFFER_SIZE, c, &cLen);
    if (!stricmp(buffer, "Y"))
        return TRUE;       // return TRUE if yes
    else if (!stricmp(buffer, "N"))
        return FALSE;
    else if (!stricmp(buffer, "timeout"))
        return 'T'; // Indicates timeout occurred
    else
    {
        sendEncrypt(sd, "\n\nUnrecognized response. Returning to main menu.", BUFFER_SIZE, 0, c, cLen);
        return FALSE;
    }
}

int seeBalanceSelected(int sd, SOCKADDR *c, int cLen, User *head)
{
    int accountNum;
    char buffer[BUFFER_SIZE];
    char result;
    User *customer;

    // Prompt user for account number
	sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
	sendEncrypt(sd, "\nYou selected option '1 - Report balance of a selected customer'\n\n", BUFFER_SIZE, 0, c, cLen);
    sendEncrypt(sd, "Account number of customer: ", BUFFER_SIZE, 0, c, cLen);
    sendDone(sd, c, cLen);

    // Receive account number
    RECV_DECRYPT(sd, buffer, BUFFER_SIZE, c, &cLen);
    if (sscanf(buffer, "%d", &accountNum) != 1)
    {
        sendEncrypt(sd, "\n\nError reading account number. Returning to main menu.", BUFFER_SIZE, 0, c, cLen);
        sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
        return 0;
    }

    // Search linked list for the selected account number
    if ( !(customer = findUserByAcctNum(head, accountNum)) )
    {
        sendEncrypt(sd, "\n\nError. Account number does not exist in the database. Returning to main menu.", BUFFER_SIZE, 0, c, cLen);
        sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
        return 0;
    }

    // Send account information and prompt for more
    sprintf(buffer, "\n\n-->Balance of account %d: $%.2f\n\nWould you like to view another account? ", customer->acctNum, (float)customer->balance/100);    // build display string
    if ((result = ynOption(buffer, sd, c, cLen)) == 'T')
        return -1;

    if (result)
        seeBalanceSelected(sd, c, cLen, head);  // call this function recursively if additional lookups requested
	else
		sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);

    return 0;
}

void seeUserBalance(int sd, SOCKADDR *c, int cLen, User *user)
{
    char accountDisplay[BUFFER_SIZE];
	
	// Dislpay selection
	sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
	sendEncrypt(sd, "\nYou selected option '1 - Report balance of the customer'\n\n", BUFFER_SIZE, 0, c, cLen);
    
	// Send account information
    sprintf(accountDisplay, "Your current balance is: $%.2f\n", (float)(user->balance)/100); // build display string
    sendEncrypt(sd, accountDisplay, BUFFER_SIZE, 0, c, cLen);                                     // send display string
    sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
}

int transferBetweenAccounts(int sd, SOCKADDR *c, int cLen, User *head)
{
    int acct1, acct2;
	int i=0, periodCnt=0;
    float transfer = 0.0, tmp;
    char buffer[BUFFER_SIZE];
    char result;
    User *customer1, *customer2;

    // Prompt user for the giving account number
	sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
	sendEncrypt(sd, "\nYou selected option '2 - Transfer money between two accounts'\n", BUFFER_SIZE, 0, c, cLen);
    sendEncrypt(sd, "\nTransfer from (account number): ", BUFFER_SIZE, 0, c, cLen);
    sendDone(sd, c, cLen);

    // Receive account number
    RECV_DECRYPT(sd, buffer, BUFFER_SIZE, c, &cLen);
    if (sscanf(buffer, "%d", &acct1) != 1)
    {
        sendEncrypt(sd, "\n\nError reading account number. Returning to main menu.", BUFFER_SIZE, 0, c, cLen);
        sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
        return 0;
    }

    // Search linked list for the selected account 1
    if ( !(customer1 = findUserByAcctNum(head, acct1)) )
    {
        sendEncrypt(sd, "\n\nError. Account number does not exist in database. Returning to main menu.", BUFFER_SIZE, 0, c, cLen);
        sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
        return 0;
    }

    // Display balance and prompt for transfer amount
    sprintf(buffer, "\n\nAccount %d has the current balance of $%.2f\n", customer1->acctNum, (float)customer1->balance/100);
    sendEncrypt(sd, buffer, BUFFER_SIZE, 0, c, cLen);
    sendEncrypt(sd, "Enter transfer amount: $", BUFFER_SIZE, 0, c, cLen);
    sendDone(sd, c, cLen);

    // Receive transfer amount
    RECV_DECRYPT(sd, buffer, BUFFER_SIZE, c, &cLen);
    if (sscanf(buffer, "%f", &transfer) != 1)
    {
        sendEncrypt(sd, "\n\nError reading transfer amount. Returning to main menu.", BUFFER_SIZE, 0, c, cLen);
        sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
        return 0;
    }
	
	// Validate transfer amount
	for (i=0; buffer[i] != '\0'; i++)
	{
		if (buffer[i] == '.')
			periodCnt++;
		else if (buffer[i]<48 || buffer[i]>57)
		{
			periodCnt=99;	// Set the count high and leave loop
			break;
		}
	}
	for (tmp = (100*transfer); tmp > 0.99; tmp--){}
	if (tmp > 0 || periodCnt > 1)	// Test if too many digits after decimal or too many periods
	{
		sendEncrypt(sd, "\n\nError. Transfer amount is invalid. Returning to main menu.", BUFFER_SIZE, 0, c, cLen);
        sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
        return 0;
	}
    else if ( (transfer*100) > customer1->balance )
    {
        sendEncrypt(sd, "\n\nError. Transfer amount is greater than customer balance. Returning to main menu.", BUFFER_SIZE, 0, c, cLen);
        sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
        return 0;
    }

    // Prompt user for the receiving account number
    sendEncrypt(sd, "\n\nTransfer to (account number): ", BUFFER_SIZE, 0, c, cLen);
    sendDone(sd, c, cLen);

    // Receive account number
    RECV_DECRYPT(sd, buffer, BUFFER_SIZE, c, &cLen);
    if (sscanf(buffer, "%d", &acct2) != 1)
    {
        sendEncrypt(sd, "\n\nError reading account number. Returning to main menu.", BUFFER_SIZE, 0, c, cLen);
        sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
        return 0;
    }

    // Search linked list for the selected account 2
    if ( !(customer2 = findUserByAcctNum(head, acct2)) )
    {
        sendEncrypt(sd, "\n\nError. Account number does not exist in database. Returning to main menu.", BUFFER_SIZE, 0, c, cLen);
        sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
        return 0;
    }

    // Confirm and complete
    sprintf(buffer, "\n\nTransfer $%.2f from account %d ($%.2f) to account %d ($%.2f)?\n", transfer, customer1->acctNum, (float)(customer1->balance)/100, customer2->acctNum, (float)(customer2->balance)/100);
    if ((result = ynOption(buffer, sd, c, cLen)) == 'T')
        return -1;
    
    if(result)
    {
        customer1->balance -= (int)(transfer*100);
        customer2->balance += (int)(transfer*100);
        sprintf(buffer, "\n\nTransferred $%.2f\nAccount %d | New balance: $%.2f\nAccount %d | New balance: $%.2f\n", transfer, customer1->acctNum, (float)(customer1->balance)/100, customer2->acctNum, (float)(customer2->balance)/100);
        sendEncrypt(sd, buffer, BUFFER_SIZE, 0, c, cLen);
		sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
    }
    else
    {
        sendEncrypt(sd, "\n\nTransaction canceled. Returning to main menu.", BUFFER_SIZE, 0, c, cLen);
        sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
    }

    return 0;
}

int transferToCustomer(int sd, SOCKADDR *c, int cLen, User *user, User *head)
{
    int acctRx, i=0, periodCnt=0;
    float transfer = 0.0, tmp;
    char buffer[BUFFER_SIZE];
    char result;
    User *userRx;

	// Dislpay selection
	sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
	sendEncrypt(sd, "\nYou selected option '2 - Transfer money to another customer account'\n\n", BUFFER_SIZE, 0, c, cLen);

    // Display balance and prompt for transfer amount
    sprintf(buffer, "Your account (%d) has the current balance of $%.2f\n", user->acctNum, (float)(user->balance)/100);
    sendEncrypt(sd, buffer, BUFFER_SIZE, 0, c, cLen);
	sendEncrypt(sd, "Enter transfer amount: $", BUFFER_SIZE, 0, c, cLen);
    sendDone(sd, c, cLen);

    // Receive transfer amount
    RECV_DECRYPT(sd, buffer, BUFFER_SIZE, c, &cLen);
    if (sscanf(buffer, "%f", &transfer) != 1)
    {
        sendEncrypt(sd, "\n\nError reading transfer amount. Returning to main menu.", BUFFER_SIZE, 0, c, cLen);
		sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
        return 0;
    }

	// Validate transfer amount
	for (i=0; buffer[i] != '\0'; i++)
	{
		if (buffer[i] == '.')
			periodCnt++;
		else if (buffer[i]<48 || buffer[i]>57)
		{
			periodCnt=2;
			break;
		}
	}
	for (tmp = (100*transfer); tmp > 0.99; tmp--){}
	if (tmp > 0 || periodCnt > 1)	// Test if too many digits after decimal or too many periods
	{
		sendEncrypt(sd, "\n\nError. Transfer amount is invalid. Returning to main menu.", BUFFER_SIZE, 0, c, cLen);
        sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
        return 0;
	}
    else if ( (transfer*100) > user->balance)
    {
        sendEncrypt(sd, "\n\nError. Transfer amount is greater than current balance. Returning to main menu.", BUFFER_SIZE, 0, c, cLen);
		sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
        return 0;
    }

    // Prompt user for the giving account number
    sendEncrypt(sd, "\n\nAccount number to receive transfer funds: ", BUFFER_SIZE, 0, c, cLen);
    sendDone(sd, c, cLen);

    // Receive account number
    RECV_DECRYPT(sd, buffer, BUFFER_SIZE, c, &cLen);
    if (sscanf(buffer, "%d", &acctRx) != 1)
    {
        sendEncrypt(sd, "\n\nError reading account number. Returning to main menu.", BUFFER_SIZE, 0, c, cLen);
		sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
        return 0;
    }
    else if ( !(userRx = findUserByAcctNum(head, acctRx)) )
    {
        sendEncrypt(sd, "\n\nError. Account number does not exist in the database. Returning to main menu.", BUFFER_SIZE, 0, c, cLen);
        sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
        return 0;
    }

    // Confirm and complete
    sprintf(buffer, "\n\nTransfer $%.2f from account %d ($%.2f) to account %d? ", transfer, user->acctNum, (float)(user->balance)/100, userRx->acctNum);
    if ((result = ynOption(buffer, sd, c, cLen)) == 'T')
        return -1;

    if(result)
    {
        user->balance -= (int)(transfer*100);
        userRx->balance += (int)(transfer*100);
        sprintf(buffer, "Transferred $%.2f\nAccount %d | New balance: $%.2f\n\n", transfer, user->acctNum, (float)(user->balance)/100);
        sendEncrypt(sd, buffer, BUFFER_SIZE, 0, c, cLen);
    }
    else
        sendEncrypt(sd, "\n\nTransaction canceled. Returning to main menu.", BUFFER_SIZE, 0, c, cLen);	

	sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);

    return 0;
}

int addNewCustomer(int sd, SOCKADDR *c, int cLen, User **head)
{
    char buffer[BUFFER_SIZE];
    char newP1[MAX_NAME_LEN], newP2[MAX_NAME_LEN];
    User *newCustomer = (User *)malloc(sizeof(User));
    float deposit;
    int pAttempts = 0;
    int uniqueUsername = 0;


    if (!newCustomer)
    {
        sendEncrypt(sd, "\n\nMemory error encountered. Returning to main menu.", BUFFER_SIZE, 0, c, cLen);
		sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
        return 0;
    }

    // Initialize
    *newCustomer = ZERO_USER;
    
	// Dislpay selection
	sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
	sendEncrypt(sd, "\nYou selected option '3 - Add a new customer to the system.'\n\n", BUFFER_SIZE, 0, c, cLen);
    
    // Prompt for new user's info
    sendEncrypt(sd, "Please enter the following information for the new user\n", BUFFER_SIZE, 0, c, cLen);
    sendEncrypt(sd, "\nFirst Name: ", BUFFER_SIZE, 0, c, cLen);
    sendDone(sd, c, cLen);
    RECV_DECRYPT(sd, newCustomer->firstName, MAX_NAME_LEN, c, &cLen);

    sendEncrypt(sd, "\nLast Name: ", BUFFER_SIZE, 0, c, cLen);
    sendDone(sd, c, cLen);
    RECV_DECRYPT(sd, newCustomer->lastName, MAX_NAME_LEN, c, &cLen);

    do
    {
        sendEncrypt(sd, "\nUsername: ", BUFFER_SIZE, 0, c, cLen);
        sendDone(sd, c, cLen);
        RECV_DECRYPT(sd, newCustomer->username, MAX_NAME_LEN, c, &cLen);

        if (!findUserByUsername(*head, newCustomer->username))
            uniqueUsername = 1;
        else
            sendEncrypt(sd, "\nThe username entered already exists. Please choose another name.\n", BUFFER_SIZE, 0, c, cLen);
    } while (!uniqueUsername);

    do
    {
	    sendEncrypt(sd, "\nPassword: ", BUFFER_SIZE, 0, c, cLen);
        sendDone(sd, c, cLen);
        RECV_DECRYPT(sd, newP1, MAX_NAME_LEN, c, &cLen);

        sendEncrypt(sd, "\nRe-enter Password: ", BUFFER_SIZE, 0, c, cLen);
        sendDone(sd, c, cLen);
        RECV_DECRYPT(sd, newP2, MAX_NAME_LEN, c, &cLen);

        if (!strcmp(newP1, newP2))
        {
            strcpy(newCustomer->password, newP1);
            break;
        }
        else
            sendEncrypt(sd, "\n\nThe passwords you entered do not match.\n\n", BUFFER_SIZE, 0, c, cLen);

        ++pAttempts;

    } while (pAttempts < 3); // Verify new password

    if (pAttempts == 3)
    {
        sendEncrypt(sd, "Returning to the main menu.", BUFFER_SIZE, 0, c, cLen);
        sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
        free(newCustomer);
        return 0;
    }

    sendEncrypt(sd, "\nInitial deposit: $", BUFFER_SIZE, 0, c, cLen);
    sendDone(sd, c, cLen);
    RECV_DECRYPT(sd, buffer, MAX_NAME_LEN, c, &cLen);
    sscanf(buffer, "%f", &deposit);
    newCustomer->balance = (unsigned int)(deposit*100);

    // Generate a random account number
    srand((unsigned)time(NULL));
    newCustomer->acctNum = 100000 + rand() / (RAND_MAX / (999998 - 100000 + 1) + 1);

    // Just in case by some strange bad luck, the acctnum exists, keep trying
    while (findUserByAcctNum(*head, newCustomer->acctNum))
        newCustomer->acctNum = 100000 + rand() / (RAND_MAX / (999998 - 100000 + 1) + 1);

    // Set type to Customer
    newCustomer->userType = 'C';

    // Add new user to list
    newCustomer->next = *head;
    *head = newCustomer;

    // Notify manager that new customer has been added
    sendEncrypt(sd, "\nThe new user has been added with the following information:\n\n", BUFFER_SIZE, 0, c, cLen);
    sprintf(buffer, "Name: %s %s\n", newCustomer->firstName, newCustomer->lastName);
    sendEncrypt(sd, buffer, BUFFER_SIZE, 0, c, cLen);

    sprintf(buffer, "Username: %s\n", newCustomer->username);
    sendEncrypt(sd, buffer, BUFFER_SIZE, 0, c, cLen);

    sprintf(buffer, "Account Number: %d\n", newCustomer->acctNum);
    sendEncrypt(sd, buffer, BUFFER_SIZE, 0, c, cLen);

    return 0;
}

int changePassword(int sd, SOCKADDR *c, int cLen, User *user)
{
	char buffer[BUFFER_SIZE];
	char newP1[MAX_NAME_LEN];
	char newP2[MAX_NAME_LEN];
	
	// Dislpay selection
	sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
	sendEncrypt(sd, "\nYou selected option '3 - Change Password'\n\n", BUFFER_SIZE, 0, c, cLen);

	// Prompt for current password
	sendEncrypt(sd, "Please enter your current password: ", BUFFER_SIZE, 0, c, cLen);   
    sendDone(sd, c, cLen);

	// Verify current password and prompt for new password
	RECV_DECRYPT(sd, buffer, MAX_NAME_LEN, c, &cLen);
    if (!strcmp(user->password, buffer))
	{
		sendEncrypt(sd, "\n\nPlease enter a new password: ", BUFFER_SIZE, 0, c, cLen);   
		sendDone(sd, c, cLen);
		RECV_DECRYPT(sd, buffer, MAX_NAME_LEN, c, &cLen);
		sscanf(buffer, "%s", newP1);

		sendEncrypt(sd, "\n\nRe-enter the new password: ", BUFFER_SIZE, 0, c, cLen);
		sendDone(sd, c, cLen);
		RECV_DECRYPT(sd, buffer, MAX_NAME_LEN, c, &cLen);
		sscanf(buffer, "%s", newP2);
	}
	else
	{
		sendEncrypt(sd, "\n\nThe password you entered is not valid. Returning to the main menu.", BUFFER_SIZE, 0, c, cLen);
		sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
		return 0;
	}

	// Verify new password
	if (!strcmp(newP1, newP2))
	{
		strcpy(user->password, newP1);
		sendEncrypt(sd, "\n\nPassword successfully changed.\n", BUFFER_SIZE, 0, c, cLen);
	}
	else
	{
		sendEncrypt(sd, "\n\nThe passwords you entered do not match. Your password is unchanged.", BUFFER_SIZE, 0, c, cLen);
	}

	sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
    return 0;
}

void totalBalanceOfAll(int sd, SOCKADDR *c, int cLen, User *head)
{
	char buffer[BUFFER_SIZE];
	double balanceSum = 0;
	unsigned int cnt = 0;
	
	// Dislpay selection
	sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
	sendEncrypt(sd, "\nYou selected option '6 - See the total balance of all customers'\n\n", BUFFER_SIZE, 0, c, cLen);

	// Calculate sum of all balances
	while(head)
    {
		balanceSum += head->balance;
		cnt++;
        head = head->next;
    }

	// Display the total
	sprintf(buffer, "The total balance of all (%d) customers is: $%.2f\n", cnt, (float)(balanceSum)/100);  // build display string
    sendEncrypt(sd, buffer, BUFFER_SIZE, 0, c, cLen);                                    // send display string
	sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
}

void listCustomer(int sd, SOCKADDR *c, int cLen, User *user)
{
    char buffer[BUFFER_SIZE];

    // Print the account information
	sprintf(buffer, "\n\nName (last, first): %s, %s\n", user->lastName, user->firstName);  // build name display string
	sendEncrypt(sd, buffer, BUFFER_SIZE, 0, c, cLen);

	sprintf(buffer, "Username: %s\n", user->username);		// build username display string
	sendEncrypt(sd, buffer, BUFFER_SIZE, 0, c, cLen);

	if(user->userType == 'M')							// build user type display string based on data
		sprintf(buffer, "User type: Manager\n");		
	else
		sprintf(buffer, "User type: Customer\n");
	sendEncrypt(sd, buffer, BUFFER_SIZE, 0, c, cLen);

	sprintf(buffer, "Account number: %u\n", user->acctNum); // build account number display string
	sendEncrypt(sd, buffer, BUFFER_SIZE, 0, c, cLen);

	sprintf(buffer, "Balance: $%.2f\n", (float)(user->balance)/100); // build balance display string
	sendEncrypt(sd, buffer, BUFFER_SIZE, 0, c, cLen);

	sprintf(buffer, "IP address: %s\n", user->ipAddr);	// build ip address display string
	sendEncrypt(sd, buffer, BUFFER_SIZE, 0, c, cLen);

	if ((long long)user->lastLogin > 0)					// build last login display string based on data
		sprintf(buffer, "Last login: %.24s\n", ctime(&user->lastLogin)); 
	else
		sprintf(buffer, "Last login: N/A\n");
	sendEncrypt(sd, buffer, BUFFER_SIZE, 0, c, cLen);
}

void listCustomers(int sd, SOCKADDR *c, int cLen, User *head)
{
	// Dislpay selection
	sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
	sendEncrypt(sd, "\nYou selected option '4 See the list of customers and their account information'", BUFFER_SIZE, 0, c, cLen);

	while(head)
    {
		listCustomer(sd, c, cLen, head);

		// Move to the next account
        head = head->next;
    }
	
	sendEncrypt(sd, BORDER, BUFFER_SIZE, 0, c, cLen);
}

// Sorts Users by balance, greatest to least, then lists them.
void sortByBalance(int sd, SOCKADDR *c, int cLen, User **head)
{
    // Only do this if there's more than one User
    if ((*head)->next)
    {
        User *newHead = *head;
        *head = (*head)->next;
        newHead->next = NULL;

        // Do this for every User
        while (*head)
        {
            User *curr = *head;
            User *insert = newHead;
            *head = (*head)->next;

            // Check edge case of curr being greater than newHead
            if (curr->balance > newHead->balance)
            {
                // Put curr in the beginning
                curr->next = newHead;
                newHead = curr;
            }

            else
            {
                // Loop until the insert position for curr is found
                for(; insert->next && curr->balance < insert->next->balance;
                    insert = insert->next) {} // Empty loop body
            
                // Put curr between insert and insert->next
                curr->next = insert->next;
                insert->next = curr;
            }
        }

        // Assign head to the head of the sorted list
        *head = newHead;
    }
}

// Unique to server because of timeout checking
int recvDecrypt(int sd, char *buffer, int bufLen, SOCKADDR *c, int *cLen)
{
	int i = recvfrom(sd, buffer, bufLen, 0, c, cLen);
	Crypt(buffer, bufLen, 1);
	
	// Check for timeout message
	if (!strcmp(buffer, "timeout"))
		return BUFFER_SIZE+1;		// just a value that recvfrom() cannot return

	return i;
}

void freeUsers(User *head)
{
    User *next = head;

    while (head)
    {
        next = head->next;
        free(head);
        head = next;
    }
}