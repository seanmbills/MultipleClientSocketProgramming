/*Included libraries*/

#include <stdio.h>    /* for printf() and fprintf() */
#include <sys/socket.h>   /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>    /* for sockaddr_in and inet_addr() */
#include <stdlib.h>   /* supports all sorts of functionality */
#include <unistd.h>   /* for close() */
#include <string.h>   /* support any string ops */
#include <time.h>       /* support the time functions to keep track of accesses to the server */
#include <pthread.h>

#define RCVBUFSIZE 2      /* The receive buffer size */
#define SNDBUFSIZE 64      /* The send buffer size */
#define BUFSIZE 40      /* Your name can be as many as 40 chars*/

int numConnections;

struct arguments {
    int socket;
    int numArgs;
    char* dictionary;
};

void lookupDictionary(char* dictionaryName) {
    FILE *fp;
    char *filename = dictionaryName;
    fp = fopen(filename, "r");

    if (fp == NULL){
        printf("Could not open file %s",filename);
        strcpy(dictionaryName, "Does not exist.");
    } else {
        if (strcmp(filename, "default.txt") == 0) {
            
            char buff[512];
            int numLines = 15;
            int r = rand() % 15;
            for (int i = 0; i < r; i++) {
                fgets(buff, sizeof(buff), fp);
            }
            fgets(buff, sizeof(buff), fp);
            strtok(buff, "\n");
            strcpy(dictionaryName, buff);

        } else {
            char line[32];
            int numLetters = 0;
            int numLines = 0;
            fscanf(fp, "%d %d", &numLetters, &numLines);
            fgets(line, 32, fp);
            int r = rand() % numLines;

            for (int i = 0; i < r; i++) {
                fgets(line, 32, fp);
            }
            fgets(line, 32, fp);
            strtok(line, "\n");
            strcpy(dictionaryName, line);
        }
    }
}

void* connection_handler(void *args) {
    ssize_t numBytes;
    char recBuf[3];

    srand(time(NULL));

    struct arguments* handlerArgs;
    handlerArgs = (struct arguments*) args;
    int clientSock = handlerArgs->socket;
    int argc = handlerArgs->numArgs;
    char* dict = handlerArgs->dictionary;

    char userWord[64];

    int startLength = 22;
    char start[23];
    strcpy(start, "Ready to start? (y/n)");
    uint8_t startMessage[1 + startLength];
    startMessage[0] = (uint8_t) startLength;
    for (int i = 0; i < startLength; i++) {
        startMessage[i + 1] = (uint8_t) start[i];
    }
    printf("Sending the welcome 'Ready to start?' message to Client %d...\n", clientSock);
    numBytes = send(clientSock, startMessage, sizeof(startMessage), 0);
    if (numBytes < 0) {
        puts("Sending the word failed...");
        numConnections--;
        printf("Number of open connections: %d\n", numConnections);
        close(clientSock);
        pthread_exit(NULL);
    }

    memset(&recBuf, 0, RCVBUFSIZE);
    numBytes = recv(clientSock, recBuf, 2, 0);
    if (numBytes < 0) {
        puts("Receiving of the word transmission failed...");
        numConnections--;
        printf("Number of open connections: %d\n", numConnections);
        close(clientSock);
        pthread_exit(NULL);
    } else if (numBytes == 0) {
        puts("Receiving connection closed prematurely...");
        numConnections--;
        printf("Number of open connections: %d\n", numConnections);
        close(clientSock);
        pthread_exit(NULL);
    }
    printf("Received a response from Client %d...\n", clientSock);
    char response = (char)recBuf[1];
    if (response == 'y') {
        puts("Client has indicated they want to play a game...");
        if (argc == 3) {
            char dictionaryLookupReturn[32];
            strcpy(dictionaryLookupReturn, dict);
            lookupDictionary(dictionaryLookupReturn);
            if (strcmp(dictionaryLookupReturn, "Does not exist.") == 0) {
                puts("A dictionary with that name does not exist.");
                puts("Defaulting to the dictionary provided in this project build.");
                lookupDictionary(dictionaryLookupReturn);
                memset(&userWord, 0, sizeof(userWord));
                strcpy(userWord, dictionaryLookupReturn);
            } else {
                puts("Found the dictionary...");
                puts("Choosing a word...");
                memset(&userWord, 0, sizeof(userWord));
                strcpy(userWord, dictionaryLookupReturn);
            }
        } else {
            puts("No dictionary name was given...");
            puts("Defaulting to the default dictionary...");
            puts("Finding a word...");
            char dictionaryLookupReturn[32];
            strcpy(dictionaryLookupReturn, "default.txt");
            lookupDictionary(dictionaryLookupReturn);
            memset(&userWord, 0, sizeof(userWord));
            strcpy(userWord, dictionaryLookupReturn);
        }

        
        int numIncorrectGuesses = 0;
        int wordLength = 0;
        char incorrectGuesses[6];
        memset(&incorrectGuesses, 0, sizeof(incorrectGuesses));
        
        int numCorrectLetters = 0;
        char guessesSoFar[64];
        int totalGuesses = 0;

        
        for (int i = 0; i < (unsigned int)sizeof(userWord); i++) {
            if (userWord[i] != 0) {
                wordLength++;
            }
        }

        char hiddenWord[wordLength];
        for (int i = 0; i < wordLength; i++) {
            strcpy(&hiddenWord[i], "_");
        }

        while (numIncorrectGuesses < 6 && numCorrectLetters != wordLength) {
            uint8_t message[3 + wordLength + numIncorrectGuesses + 1];
            message[0] = (uint8_t) 0;
            message[1] = (uint8_t) wordLength;
            message[2] = (uint8_t) numIncorrectGuesses;
          
            for (int i = 3; i < (wordLength + 3); i++) {
                message[i] = (uint8_t) hiddenWord[i - 3];
            }

            for (int i = 0; i < numIncorrectGuesses; i++) {
                message[i + 3 + wordLength] = (uint8_t) incorrectGuesses[i];
            }
            
            printf("Sending the message to Client %d...\n", clientSock);
            numBytes = send(clientSock, message, sizeof(message), 0);
            if (numBytes < 0) {
                puts("Sending the word failed...");
                numConnections--;
                printf("Number of open connections: %d\n", numConnections);
                close(clientSock);
                pthread_exit(NULL);
            }


            // printf("Preparing to receive a response from Client %d...\n", clientSock);
            memset(&recBuf, 0, RCVBUFSIZE);
            numBytes = recv(clientSock, recBuf, 3, 0);
            if (numBytes < 0) {
                puts("Receiving of the word transmission failed...");
                numConnections--;
                printf("Number of open connections: %d\n", numConnections);
                close(clientSock);
                pthread_exit(NULL);
            } else if (numBytes == 0) {
                puts("Receiving connection closed prematurely...");
                numConnections--;
                printf("Number of open connections: %d\n", numConnections);
                close(clientSock);
                pthread_exit(NULL);
            }
            // printf("Message received from Client %d...\n", clientSock);

            int msgFlag = (int)recBuf[0];
            char userGuess = (char) recBuf[1];
            
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
                    for (int i = 0; i < numIncorrectGuesses; i++) {
                        if (incorrectGuesses[i] == userGuess) {
                            notGuessedThisIncorrectYet = 0;
                        }
                    }
                    if (notGuessedThisIncorrectYet == 1) {
                        incorrectGuesses[numIncorrectGuesses] = userGuess;
                        numIncorrectGuesses++;
                    }
                }
            }

            
        }
        if (numCorrectLetters == wordLength) {
            printf("Client %d has won their game!\n", clientSock);
            char win[9];
            strcpy(win, "You Win!");
            uint8_t message[1 + sizeof(win)];
            message[0] = (uint8_t) 8;
            for (int i = 0; i < ((unsigned int) sizeof(win)); i++) {
                message[i + 1] = (uint8_t) win[i];
            }
            numBytes = send(clientSock, message, sizeof(message), 0);
            if (numBytes < 0) {
                puts("Sending the message 'You Win!' failed...");
                close(clientSock);
                pthread_exit(NULL);
            }
            printf("Closing connection with Client %d...\n", clientSock);
            numConnections--;
            printf("Number of open connections: %d\n", numConnections);
        } else if (numIncorrectGuesses == 6) {
            printf("Client %d has lost their game...\n", clientSock);
            char lose[10];
            strcpy(lose, "Game Over");
            uint8_t message[1 + sizeof(lose)];
            message[0] = (uint8_t) 9;
            for (int i = 0; i < ((unsigned int) sizeof(lose)); i++) {
                message[i + 1] = (uint8_t) lose[i];
            }
            numBytes = send(clientSock, message, sizeof(message), 0);
            if (numBytes < 0) {
                puts("Sending the message 'Game Over' failed...");
                close(clientSock);
                pthread_exit(NULL);
            }
            printf("Closing connection with Client %d...\n", clientSock);
            numConnections--;
            printf("Number of open connections: %d\n", numConnections);
        }
    } else {
        puts("User has indicated they do not wish to begin a game...");
        printf("Closing connection with Client %d...\n", clientSock);
        numConnections--;
        printf("Number of open connections: %d\n", numConnections);
        close(clientSock);
        pthread_exit(NULL);
    }

    puts("Closing the thread in which this Hangman game is running...");
    pthread_exit(NULL);
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


       /* variable to hold the number of bytes sent/received */
                        /* for verification purposes */

    pthread_t threads[3];

    numConnections = 0;

    


    if (argc < 2 || argc > 3) {
        puts("Incorrect number of arguments. The correct format is: serverPort     dictionary.txt (optional)");
        exit(1);
    }
    unsigned short port_num = atoi(argv[1]);
    

    /* Create new TCP Socket for incoming requests*/
    /*      FILL IN */
    puts("Creating new TCP socket...");
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
    puts("Binding the socket to the local address structure...");
    if (bind(serverSock, (struct sockaddr *) &changeServAddr, sizeof(changeServAddr)) < 0) {
        printf("Binding of the local address structure failed...\n");
        close(serverSock);
        exit(1);
    }

    /* Listen for incoming connections */
    /*      FILL IN */
    puts("Listening on the socket...");
    if (listen(serverSock, 3) < 0) {
        printf("Listening to the socket failed...\n");
        close(serverSock);
        exit(1);
    }

    ssize_t numBytes;

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
        puts("Accepting a new connection...");
        numConnections++;
        if (numConnections <= 3) {
            
            struct arguments* args;
            args->socket = clientSock;
            args->numArgs = argc;
            args->dictionary = argv[2];

            puts("Creating a new thread for this new connection to run in...");
            pthread_create(&threads[numConnections - 1], NULL, connection_handler, args);

            
            printf("New Connection! Num Connections: %d\n", numConnections);


        } else {
            puts("server-overload");
            puts("The server is currently full and has alerted the new attempted connection of this fact...");
            char overload[18];
            strcpy(overload, "server-overload");
            uint8_t message[1 + sizeof(overload)];
            message[0] = (uint8_t) 17;
            for (int i = 0; i < sizeof(overload); i++) {
                message[i + 1] = (uint8_t) overload[i];
            }
            numBytes = send(clientSock, message, sizeof(message), 0);
            if (numBytes < 0) {
                puts("Sending the message 'server-overload' failed...");
                close(clientSock);
                exit(1);
            }
            printf("Closing connection with Client %d...\n", clientSock);
            numConnections--;
            printf("Number of open connections: %d\n", numConnections);
            close(clientSock);
        }
    }
}