/*///////////////////////////////////////////////////////////
*
* FILE:     client.c
* AUTHOR:   Sean M Bills 
* PROJECT:  CS 3251 Project 1 - Professor Ellen Zegura 
* DESCRIPTION:  Network Client Code
* CREDIT:   Adapted from Professor Traynor
*
*////////////////////////////////////////////////////////////

/* Included libraries */

#include <stdio.h>          /* for printf() and fprintf() */
#include <sys/socket.h>         /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>          /* for sockaddr_in and inet_addr() */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* Constants */
#define RCVBUFSIZE 64          /* The receive buffer size */
#define SNDBUFSIZE 64          /* The send buffer size */
#define REPLYLEN 32
#define WORDLEN 64

void determineServerMessage(char* response) {
    char flag = response[0];
    int msgFlag = atoi(&flag);
    char output[msgFlag + 1];
    memset(&output, 0, sizeof(output));
    for (int i = 1; i <= msgFlag; i++) {
        output[i - 1] = response[i];
    }
    if (strcmp(output, "server-overload") == 0) {
        puts(output);
        puts("The server is currently full. Please try again later.");
    } else if (strcmp(output, "You Win!") == 0) {
        puts("Congratulations! You Win!");
    } else if (strcmp(output, "Game Over") == 0) {
        puts("Game Over! Better luck next time.");
    }
}

void handleWordCase(char* response) {
    char flag = response[0];
    int messageFlag = atoi(&flag);
    char length = response[1];
    int wordLength = atoi(&length);
    char incorrect = response[2];
    int numIncorrectGuesses = atoi(&incorrect);
    int totalResponseLength = (unsigned int)sizeof(response);
    char encodedWord[wordLength];
    for (int i = 0; i < wordLength; i++) {
        encodedWord[i] = response[3 + i];
    }

    printf("The word is: ");
    for (int i = 0; i < sizeof(encodedWord); i++) {
        printf("%c ", encodedWord[i]);
    }
    puts("");
    printf("Number of incorrect guesses remaining: %i\n", 6 - numIncorrectGuesses);
    printf("Incorrect guesses so far: ");
    if (numIncorrectGuesses != 0) {
        for (int i = 0; i < numIncorrectGuesses; i++) {
            printf("%c", response[i + 3 + wordLength]);
            printf(", ");
        }
    } else {
        printf("None");
    }
    printf("\n");
}

/* The main function */
int main(int argc, char *argv[])
{

    int clientSock;         /* socket descriptor */
    struct sockaddr_in serv_addr;   /* server address structure */

    char *servIP;                   /* Server IP address  */
    unsigned short servPort;        /* Server Port number */
    

    char sndBuf[SNDBUFSIZE];        /* Send Buffer */
    char rcvBuf[RCVBUFSIZE];        /* Receive Buffer */
    
    int balance;            /* Account balance */

    /* Get the Account Name from the command line */
    if (argc != 3) {
        printf("Incorrect number of arguments. The correct format is: serverIP serverPort");
        exit(1);
    }
    servIP = argv[1];
    unsigned short port_num = atoi(argv[2]);

    memset(&sndBuf, 0, SNDBUFSIZE);
    memset(&rcvBuf, 0, RCVBUFSIZE);

    ssize_t numBytes;                   /* variable to hold the number of bytes sent/received */
                                        /* for verification purposes */

    /* Create a new TCP socket*/
    /*      FILL IN */

    clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSock < 0) {
        puts("Could not create socket...");
        close(clientSock);
        exit(1);
    }

    /* Construct the server address structure */
    /*      FILL IN  */

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(servIP);
    serv_addr.sin_port = htons(port_num);

    /* Establish connecction to the server */
    /*      FILL IN  */

    if (connect(clientSock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("Could not connect to server...\n");
        close(clientSock);
        exit(1);
    }
    
    /* Send the string to the server */
    /*      FILL IN  */

    // create variables to hold server responses
    int numLetters = 0;
    char serverResponse[64];
    // char userGuess[2];
    char userGuess;

    
    while (1) {
        // clear out the receive buffer to receive the initial indication from the server
        // as to how big the word is
        memset(&rcvBuf, 0, RCVBUFSIZE);
        // receive the initial response from the server in regards to the word the
        // client is intended to guess
        numBytes = recv(clientSock, rcvBuf, RCVBUFSIZE, 0);
        // puts(rcvBuf);
        if (numBytes < 0) {
            puts("Receiving of the word transmission failed...");
            close(clientSock);
            exit(1);
        } else if (numBytes == 0) {
            puts("Receiving connection closed prematurely...");
            close(clientSock);
            exit(1);
        }


        // the response format for the server defines that the first
        // cell of the array of bytes should be the length of the response
        // thus we can pull the number of letters in the word from this response
        char messageFlag = rcvBuf[0];
        int msgFlag = atoi(&messageFlag);
        

        for (int i = 0; i < WORDLEN; i++) {
            serverResponse[i] = rcvBuf[i];
        }
        // puts(serverResponse);

        if (msgFlag != 0) {
            determineServerMessage(serverResponse);
            close(clientSock);
            break;
        } else {
            fflush(stdin);
            handleWordCase(serverResponse);
            printf("Please guess a letter: \n");
            userGuess = getchar();
            // printf("%c\n", userGuess);
            char c = getchar();
            
            char userMessage[2];
            sprintf(&userMessage[0], "1");
            userMessage[1] = userGuess;
            // printf("%d", (unsigned int)sizeof(userMessage));
            numBytes = send(clientSock, userMessage, sizeof(userMessage), 0);
            // printf("%d", (unsigned int) numBytes);
            if (numBytes < 0) {
                puts("Sending the account name failed...");
                close(clientSock);
                exit(1);
            }
        }
    }
}