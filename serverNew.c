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

// a global variable to keep track of the number of connections
// currently running on the server
int numConnections;


// a struct to use to pass in useful information to the thread
// connection_handler function upon creation of new threads
// for games to play in
struct arguments {
    int socket;
    int numArgs;
    char* dictionary;
};


// a function intended to lookup words in the given dictionary file
// in order to find words to play Hangman with
void lookupDictionary(char* dictionaryName) {
    FILE *fp;
    char *filename = dictionaryName;
    fp = fopen(filename, "r");

    // check if the file name passed in actually exists
    if (fp == NULL){
        printf("Could not open file %s",filename);
        strcpy(dictionaryName, "Does not exist.");
    } else {
        // otherwise check if the file we're looking for is the default
        // dictionary file
        if (strcmp(filename, "default.txt") == 0) {
            
            char buff[512];
            int numLines = 15;
            // generate a random number between 0 and the number
            // of lines in our default dictionary (15)
            int r = rand() % 15;
            // move through the file until we get to the line
            // prior to the one we want
            for (int i = 0; i < r; i++) {
                fgets(buff, sizeof(buff), fp);
            }
            // get the line in the file we want
            fgets(buff, sizeof(buff), fp);
            strtok(buff, "\n");
            // copy the word into the address of the dictionary lookup
            // we've created to be used in the connection_handler
            strcpy(dictionaryName, buff);

        } else {
            // otherwise, open the file and and parse through it for the desired word
            // works the same as above
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


// a method intended to handle all of the functionality each thread needs
// to play through an individual game of Hangman separately but concurrently
void* connection_handler(void *args) {
    // a variable to make sure that sending/receiving is occurring correctly
    ssize_t numBytes;
    // a buffer to receive the messages from the Client in
    char recBuf[3];

    // a seed for the random number generators used in each thread
    srand(time(NULL));

    // get all of the relevant information for this thread
    //      such as the socket number for the client connection
    //      and the dictionary passed in to the command line (if one exists)
    struct arguments* handlerArgs;
    handlerArgs = (struct arguments*) args;
    int clientSock = handlerArgs->socket;
    int argc = handlerArgs->numArgs;
    char* dict = handlerArgs->dictionary;

    // create a char* to store the word for the user to guess in
    char userWord[64];

    // information about the Ready to start message sent to the Client
    int startLength = 22;
    char start[23];
    strcpy(start, "Ready to start? (y/n)");
    uint8_t startMessage[1 + startLength];
    startMessage[0] = (uint8_t) startLength;
    for (int i = 0; i < startLength; i++) {
        startMessage[i + 1] = (uint8_t) start[i];
    }
    printf("Sending the welcome 'Ready to start?' message to Client %d...\n", clientSock);
    // send the Ready to start message
    numBytes = send(clientSock, startMessage, sizeof(startMessage), 0);
    if (numBytes < 0) {
        puts("Sending the word failed...");
        numConnections--;
        printf("Number of open connections: %d\n", numConnections);
        close(clientSock);
        pthread_exit(NULL);
    }

    // clear the receiving buffer
    memset(&recBuf, 0, RCVBUFSIZE);
    // receive the response from the Client
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

    // get the Client's response (y/n)
    char response = (char)recBuf[1];

    // if the Client wishes to start a game
    if (response == 'y') {
        puts("Client has indicated they want to play a game...");
        // check the number of arguments provided to the command line
        if (argc == 3) {
            char dictionaryLookupReturn[32];
            strcpy(dictionaryLookupReturn, dict);

            // perform a dictionary lookup
            lookupDictionary(dictionaryLookupReturn);

            // if our lookup finds that this file does not exist
            if (strcmp(dictionaryLookupReturn, "Does not exist.") == 0) {
                puts("A dictionary with that name does not exist.");
                puts("Defaulting to the dictionary provided in this project build.");
                // then default to the default dictionary
                lookupDictionary("dictionary.txt");
                memset(&userWord, 0, sizeof(userWord));
                // copy the user word in to the address of the return
                strcpy(userWord, dictionaryLookupReturn);
            } else {
                // otherwise use the given dictionary name if it exists
                puts("Found the dictionary...");
                puts("Choosing a word...");
                memset(&userWord, 0, sizeof(userWord));
                strcpy(userWord, dictionaryLookupReturn);
            }
        } else {
            // if no dictionary name was provided then we have to use the default dictionary
            puts("No dictionary name was given...");
            puts("Defaulting to the default dictionary...");
            puts("Finding a word...");
            char dictionaryLookupReturn[32];
            strcpy(dictionaryLookupReturn, "default.txt");
            lookupDictionary(dictionaryLookupReturn);
            memset(&userWord, 0, sizeof(userWord));
            strcpy(userWord, dictionaryLookupReturn);
        }

        
        /* numerous variables to keep track of:
            -- the number of incorrect guesses made by the client so far
            -- the length of the word for the client to guess
            -- the incorrect letters that the user has guessed so far
        */
        int numIncorrectGuesses = 0;
        int wordLength = 0;
        char incorrectGuesses[6];
        memset(&incorrectGuesses, 0, sizeof(incorrectGuesses));
        
        /* variables to keep track of:
            -- the number of correct letters guessed so far (to verify when a user has guessed all letters)
            -- the letters a user has guessed so far (to make sure user doesn't guess same letter again...actually handled by client now)
            -- the total number of guesses that the user has made thus far
        */
        int numCorrectLetters = 0;
        char guessesSoFar[64];
        int totalGuesses = 0;

        
        // iterate through and calculate the length of the word for the user to guess
        for (int i = 0; i < (unsigned int)sizeof(userWord); i++) {
            if (userWord[i] != 0) {
                wordLength++;
            }
        }

        // create a "hidden" word that is the user word but with '_' in place of letters
        // this will get updated each time a user guesses a correct letter
        char hiddenWord[wordLength];
        for (int i = 0; i < wordLength; i++) {
            strcpy(&hiddenWord[i], "_");
        }

        // while the user has not guessed 6 incorrect letters
        // or guessed all of the correct letters
        while (numIncorrectGuesses < 6 && numCorrectLetters != wordLength) {
            // create a message with the msgFlag at the front
            // the word length in the second byte
            // the number of incorrect guesses so far in the 3 byte
            uint8_t message[3 + wordLength + numIncorrectGuesses + 1];
            message[0] = (uint8_t) 0;
            message[1] = (uint8_t) wordLength;
            message[2] = (uint8_t) numIncorrectGuesses;
          
            // add in the "encoded" word to the message
            for (int i = 3; i < (wordLength + 3); i++) {
                message[i] = (uint8_t) hiddenWord[i - 3];
            }

            // if there are incorrect guesses so far, add them in as well
            for (int i = 0; i < numIncorrectGuesses; i++) {
                message[i + 3 + wordLength] = (uint8_t) incorrectGuesses[i];
            }
            

            printf("Sending the message to Client %d...\n", clientSock);
            // send the message to the client
            numBytes = send(clientSock, message, sizeof(message), 0);
            if (numBytes < 0) {
                puts("Sending the word failed...");
                numConnections--;
                printf("Number of open connections: %d\n", numConnections);
                close(clientSock);
                pthread_exit(NULL);
            }


            memset(&recBuf, 0, RCVBUFSIZE);
            // receive the client's letter guess in response
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

            // break down the response into the msgFlag (should always be 1)
            // and the user's letter guess (should always be the second character)
            int msgFlag = (int)recBuf[0];
            char userGuess = (char) recBuf[1];
            

            // check whether the user has guessed a letter so far
            // this is also handled on the Client side but a second check doesn't hurt
            int notGuessedYet = 1;
            for (int i = 0; i < sizeof(guessesSoFar); i++) {
                if (strcmp(&guessesSoFar[i], &userGuess) == 0) {
                    notGuessedYet = 0;
                }
            }
            
            // if the user has not already guessed a letter, then check if the guessed letter
            // exists within the word for them to guess
            if (notGuessedYet == 1) {
                int correctGuess = 0;
                // make the check about whether the letter is in the user's word to guess
                for (int i = 0; i < wordLength; i++) {
                    if (userWord[i] == userGuess) {
                        hiddenWord[i] = userGuess;
                        correctGuess = 1;
                        numCorrectLetters++;
                    }
                }
                // if the user has now guessed all the letters in the word, break out
                // and send the game won message
                if (numCorrectLetters == wordLength) {
                    break;
                }

                // keep track of the incorrect guesses so far
                // if the letter guessed is not in the word, then add it to the
                // list of incorrect guesses
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

        // check if the user has officially guessed all of the correct letters in the word
        if (numCorrectLetters == wordLength) {
            printf("Client %d has won their game!\n", clientSock);
            
            // if so, set up a message to send to the client informing them that they've won
            char win[9];
            strcpy(win, "You Win!");
            uint8_t message[1 + sizeof(win)];
            message[0] = (uint8_t) 8;
            for (int i = 0; i < ((unsigned int) sizeof(win)); i++) {
                message[i + 1] = (uint8_t) win[i];
            }

            // send the game won message
            numBytes = send(clientSock, message, sizeof(message), 0);
            if (numBytes < 0) {
                puts("Sending the message 'You Win!' failed...");
                close(clientSock);
                pthread_exit(NULL);
            }

            // break down the client connection
            printf("Closing connection with Client %d...\n", clientSock);
            numConnections--;
            printf("Number of open connections: %d\n", numConnections);
        } else if (numIncorrectGuesses == 6) {
            // if, however, the user has made 6 incorrect guesses to date, then
            // prepare a message to inform them they've lost the game

            printf("Client %d has lost their game...\n", clientSock);
            char lose[10];
            strcpy(lose, "Game Over");
            uint8_t message[1 + sizeof(lose)];
            message[0] = (uint8_t) 9;
            for (int i = 0; i < ((unsigned int) sizeof(lose)); i++) {
                message[i + 1] = (uint8_t) lose[i];
            }

            // send the game over, you lost message to the client
            numBytes = send(clientSock, message, sizeof(message), 0);
            if (numBytes < 0) {
                puts("Sending the message 'Game Over' failed...");
                close(clientSock);
                pthread_exit(NULL);
            }
            // reak down the client connection
            printf("Closing connection with Client %d...\n", clientSock);
            numConnections--;
            printf("Number of open connections: %d\n", numConnections);
        }
    } else {
        // if the user indicates they don't want to play the game by inputting an n
        puts("User has indicated they do not wish to begin a game...");
        printf("Closing connection with Client %d...\n", clientSock);

        // then break down the client connection
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

    // if the command line does not have the correct number of inputs entered, inform them
    // and close down the server
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

        // accept a new connection, increment the number of connections, then check if this new number
        // is more than the specified limit (aka 3 connections)
        puts("Accepting a new connection...");
        numConnections++;
        if (numConnections <= 3) {
            
            // set up a struct of necessary information to send to the 
            // connection_handler when a thread is made
            struct arguments* args;
            args->socket = clientSock;
            args->numArgs = argc;
            args->dictionary = argv[2];

            // create a new thread for this game to run in 
            puts("Creating a new thread for this new connection to run in...");
            pthread_create(&threads[numConnections - 1], NULL, connection_handler, args);

            
            printf("New Connection! Num Connections: %d\n", numConnections);


        } else {
            // if there are now more than 3 connections, then inform the client of
            // the apparent server-overload
            puts("server-overload");
            puts("The server is currently full and has alerted the new attempted connection of this fact...");
            
            // copy this message into a byte array
            char overload[18];
            strcpy(overload, "server-overload");
            uint8_t message[1 + sizeof(overload)];
            message[0] = (uint8_t) 17;
            for (int i = 0; i < sizeof(overload); i++) {
                message[i + 1] = (uint8_t) overload[i];
            }

            // send the byte array message
            numBytes = send(clientSock, message, sizeof(message), 0);
            if (numBytes < 0) {
                puts("Sending the message 'server-overload' failed...");
                close(clientSock);
                exit(1);
            }

            // break down the client connection and decrement our number of connections
            printf("Closing connection with Client %d...\n", clientSock);
            numConnections--;
            printf("Number of open connections: %d\n", numConnections);
            close(clientSock);
        }
    }
}