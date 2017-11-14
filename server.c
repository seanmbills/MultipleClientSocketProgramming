/*///////////////////////////////////////////////////////////
*
* FILE:     server.c
* AUTHOR:   Sean M Bills
* PROJECT:  CS 3251 Project 1 - Professor Ellen Zegura 
* DESCRIPTION:  Network Server Code
* CREDIT:   Adapted from Professor Traynor
*
*////////////////////////////////////////////////////////////

/*Included libraries*/

#include <stdio.h>    /* for printf() and fprintf() */
#include <sys/socket.h>   /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>    /* for sockaddr_in and inet_addr() */
#include <stdlib.h>   /* supports all sorts of functionality */
#include <unistd.h>   /* for close() */
#include <string.h>   /* support any string ops */
#include <time.h>       /* support the time functions to keep track of accesses to the server */

#define RCVBUFSIZE 2      /* The receive buffer size */
#define SNDBUFSIZE 64      /* The send buffer size */
#define BUFSIZE 40      /* Your name can be as many as 40 chars*/


void lookupDictionary(char* dictionaryName) {
    FILE *fp;
    char *filename = dictionaryName;
    fp = fopen(filename, "r");

    if (fp == NULL){
        printf("Could not open file %s",filename);
        strcpy(dictionaryName, "Does not exist.");
    } else {
        char line[32];
        int numLetters = 0;
        int numLines = 0;
        //fgets(line, 32, fp);
        fscanf(fp, "%d %d", &numLetters, &numLines);
        // printf("Num Letters: %d\n", numLetters);
        // printf("Num Lines: %d", numLines);
        fgets(line, 32, fp);
        int r = rand() % numLines;

        // printf("Random: %d\n", r);
        for (int i = 0; i < r; i++) {
            fgets(line, 32, fp);
        }
        fgets(line, 32, fp);
        // printf("%s", line);
        strtok(line, "\n");
        strcpy(dictionaryName, line);
    }
}


int main(int argc, char *argv[])
{

    int serverSock;             /* Server Socket */
    int clientSock;             /* Client Socket */
    struct sockaddr_in changeServAddr;      /* Local address */
    struct sockaddr_in changeClntAddr;      /* Client address */
    unsigned short changeServPort;      /* Server port */
    unsigned int clntLen;           /* Length of address data struct */

    char nameBuf[BUFSIZE];          /* Buff to store account name from client */
    int  balance;               /* Place to record account balance result */


    ssize_t numBytes;   /* variable to hold the number of bytes sent/received */
                        /* for verification purposes */

    int numConnections = 0;

    char recBuf[2];


    if (argc < 2 || argc > 3) {
        puts("Incorrect number of arguments. The correct format is: serverPort     dictionary.txt (optional)");
        exit(1);
    }
    unsigned short port_num = atoi(argv[1]);
    

    /* Create new TCP Socket for incoming requests*/
    /*      FILL IN */

    serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSock < 0) {
        printf("Failed to create the socket...\n");
        close(serverSock);
        exit(1);
    }

    /* Construct local address structure*/
    /*      FILL IN */
    
    memset(&changeServAddr, 0, sizeof(changeServAddr));
    changeServAddr.sin_family = AF_INET;
    changeServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    changeServAddr.sin_port = htons(port_num);

    /* Bind to local address structure */
    /*      FILL IN */

    if (bind(serverSock, (struct sockaddr *) &changeServAddr, sizeof(changeServAddr)) < 0) {
        printf("Binding of the local address structure failed...\n");
        close(serverSock);
        exit(1);
    }

    /* Listen for incoming connections */
    /*      FILL IN */

    if (listen(serverSock, 3) < 0) {
        printf("Listening to the socket failed...\n");
        close(serverSock);
        exit(1);
    }

    /* Loop server forever*/
    while(1)
    {
        /* Accept incoming connection */
        /*  FILL IN     */

        clntLen = sizeof(changeClntAddr);
        clientSock = accept(serverSock, (struct sockaddr *) &changeClntAddr, &clntLen);
        if (clientSock < 0) {
            printf("Accepting the incoming connection failed...\n");
            close(clientSock);
            close(serverSock);
            exit(1);
        }

        numConnections++;
        printf("%d\n", numConnections);

        if (numConnections <= 3) {
            int pid = fork();
            if (pid == 0) {
                char userWord[64];

                if (argc == 3) {
                    char dictionaryLookupReturn[32];
                    strcpy(dictionaryLookupReturn, argv[2]);
                    lookupDictionary(dictionaryLookupReturn);
                    if (strcmp(dictionaryLookupReturn, "Does not exist.") == 0) {
                        puts("A dictionary with that name does not exist.");
                        puts("Defaulting to the dictionary provided in this project build.");
                        memset(&userWord, 0, sizeof(userWord));
                        strcpy(userWord, "hangman");
                    } else {
                        // puts(dictionaryLookupReturn);
                        memset(&userWord, 0, sizeof(userWord));
                        strcpy(userWord, dictionaryLookupReturn);
                    }
                } else {
                    memset(&userWord, 0, sizeof(userWord));
                    strcpy(userWord, "hangman");
                }

                // puts("accepting connection now...");

                // create variables to track number of incorrect guesses, keep track of the
                // hidden and found letters
                int numIncorrectGuesses = 0;
                int wordLength = 0;
                char incorrectGuesses[7];
                memset(&incorrectGuesses, 0, sizeof(incorrectGuesses));
                
                int numCorrectLetters = 0;
                char guessesSoFar[64];
                int totalGuesses = 0;

                
                // puts(userWord);
                for (int i = 0; i < (unsigned int)sizeof(userWord); i++) {
                    // printf("%c", userWord[i]);
                    if (userWord[i] != 0) {
                        wordLength++;
                    }
                }
                // printf("Word Length: %d\n", wordLength);

                char hiddenWord[wordLength];
                for (int i = 0; i < wordLength; i++) {
                    strcpy(&hiddenWord[i], "_");
                }
                // puts(hiddenWord);

                while (numIncorrectGuesses < 6 && numCorrectLetters != wordLength) {
                    char message[3 + wordLength + numIncorrectGuesses + 1];
                    sprintf(&message[0], "0");
                    snprintf(&message[1], sizeof(wordLength), "%i", wordLength);
                    snprintf(&message[2], sizeof(numIncorrectGuesses), "%i", numIncorrectGuesses);
                    
                    for (int i = 0; i < wordLength; i++) {
                        strcpy(&message[i + 3], &hiddenWord[i]);
                    }

                    for (int i = 0; i < numIncorrectGuesses; i++) {
                        strcpy(&message[3 + wordLength + i], &incorrectGuesses[i]);
                    }
                    
                    // puts(message);

                    numBytes = send(clientSock, message, SNDBUFSIZE, 0);
                    if (numBytes < 0) {
                        puts("Sending the word failed...");
                        close(clientSock);
                        exit(1);
                    }

                    memset(&recBuf, 0, RCVBUFSIZE);
                    numBytes = recv(clientSock, recBuf, 2, 0);
                    // printf("%d", (unsigned int)numBytes);
                    // puts(recBuf);
                    if (numBytes < 0) {
                        puts("Receiving of the word transmission failed...");
                        close(clientSock);
                        exit(1);
                    } else if (numBytes == 0) {
                        puts("Receiving connection closed prematurely...");
                        close(clientSock);
                        exit(1);
                    }

                    char userGuess = recBuf[1];
                    
                    int notGuessedYet = 1;
                    for (int i = 0; i < sizeof(guessesSoFar); i++) {
                        if (strcmp(&guessesSoFar[i], &userGuess) == 0) {
                            notGuessedYet = 0;
                        }
                    }
                    
                    if (notGuessedYet == 1) {
                        int correctGuess = 0;
                        for (int i = 0; i < wordLength; i++) {
                            if (userWord[i] == userGuess) {
                                hiddenWord[i] = userGuess;
                                correctGuess = 1;
                                numCorrectLetters++;
                            }
                        }
                        if (numCorrectLetters == wordLength) {
                            break;
                        }
                        if (correctGuess == 0) {
                            int notGuessedThisIncorrectYet = 1;
                            for (int i = 0; i < sizeof(incorrectGuesses); i++) {
                                if (incorrectGuesses[i] == userGuess) {
                                    notGuessedThisIncorrectYet = 0;
                                }
                            }
                            if (notGuessedThisIncorrectYet == 1) {
                                incorrectGuesses[numIncorrectGuesses] = userGuess;
                                // printf("%c\n", incorrectGuesses[numIncorrectGuesses]);
                                numIncorrectGuesses++;
                            }
                        }
                    }

                    
                }
                if (numCorrectLetters == wordLength) {
                    char message[10];
                    sprintf(&message[0], "8");
                    snprintf(&message[1], sizeof("You Win!"), "%s", "You Win!");
                    numBytes = send(clientSock, message, sizeof(message), 0);
                    if (numBytes < 0) {
                        puts("Sending the message 'You Win!' failed...");
                        close(clientSock);
                        exit(1);
                    }
                    // break;
                } else if (numIncorrectGuesses == 6) {
                    char message[11];
                    sprintf(&message[0], "9");
                    snprintf(&message[1], sizeof("Game Over"), "%s", "Game Over");
                    numBytes = send(clientSock, message, sizeof(message), 0);
                    if (numBytes < 0) {
                        puts("Sending the message 'Game Over' failed...");
                        close(clientSock);
                        exit(1);
                    }
                    // break;
                }
            }
        } else {
            puts("too many connections...");
            char message[18];
            sprintf(&message[0], "17");
            snprintf(&message[1], sizeof("server-overload"), "%s", "server-overload");
            numBytes = send(clientSock, message, sizeof(message), 0);
            if (numBytes < 0) {
                puts("Sending the message 'server-overload' failed...");
                close(clientSock);
                exit(1);
            }
        }
    }
}