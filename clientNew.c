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

// a function intended to determine whether the client has
// received one of the defined messages (game over or server-overload)
// and if so, print out information relevant to them
void determineServerMessage(int flag, char* response) {
    char message[flag + 1];
    for (int i = 0; i < flag; i++) {
        strcpy(&message[i], &response[i]);
    }
    if (strcmp(message, "server-overload") == 0) {
        puts(message);
        puts("The server is currently full. Please try again later.");
    } else if (strcmp(message, "You Win!") == 0) {
        puts("Congratulations! You Win!");
        puts("Game Over!");
    } else if (strcmp(message, "Game Over") == 0) {
        puts("You lose :( Better luck next time!");
        puts("Game Over!");
    }
}


// a function intended to handle messages sent to the client 
// and print out the information sent to the client
// namely the partially encoded word, the number of incorrect guesses
// remaining, and the incorrect guesses made so far
void handleWordCase(int flag, int length, int incorrect, char* word, char* guesses) {
    printf("The word is: ");
    for (int i = 0; i < length; i++) {
        printf("%c ", word[i]);
    }
    puts("");
    printf("Number of incorrect guesses remaining: %i\n", 6 - incorrect);
    printf("Incorrect guesses so far: ");
    if (incorrect != 0) {
        for (int i = 0; i < incorrect; i++) {
            printf("%c", ((char)guesses[i]) - 32);
            printf(", ");
        }
    } else {
        printf("None");
    }
    printf("\n");
}


// a function intended to print out the ready to start question
// if it's passed in
void handleStartCase(int flag, char* question) {
    char answer[32];
    char output;
    if (strcmp(question, "Ready to start? (y/n)") == 0) {
        printf("%s\n", question);
    }
}

/* The main function */
int main(int argc, char *argv[])
{

    int clientSock;                 /* socket descriptor */
    struct sockaddr_in serv_addr;   /* server address structure */

    char *servIP;                   /* Server IP address  */
    unsigned short servPort;        /* Server Port number */
    

    char sndBuf[SNDBUFSIZE];        /* Send Buffer */
    char rcvBuf[RCVBUFSIZE];        /* Receive Buffer */

    char guessedLetters[64];
    int numLettersGuessed = 0;

    // make sure the user enters the correct number of arguments
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

    
    memset(&rcvBuf, 0, sizeof(rcvBuf));
    // receive the initial ready to start question from the server
    // or the server overload message, depending on the situation
    numBytes = recv(clientSock, rcvBuf, RCVBUFSIZE, 0);
    if (numBytes < 0) {
        puts("Receiving of the word transmission failed...");
        close(clientSock);
        exit(1);
    } else if (numBytes == 0) {
        puts("Receiving connection closed prematurely...");
        close(clientSock);
        exit(1);
    }
    // break down the information received in the rcvBuf
    // and use it to determine the message received
    char output;
    int flag = (int)rcvBuf[0];
    char startQuestion[flag + 1];
    for (int i = 0; i < flag; i++) {
        startQuestion[i] = (char)rcvBuf[i + 1];
    }
    if (strcmp(startQuestion, "server-overload") == 0) {
        determineServerMessage(flag, startQuestion);
        close(clientSock);
        exit(1);
    }
    // if the server message received wasn't a game over or server-overload message
    // then it had to be a start message
    handleStartCase(flag, startQuestion);

    // get user's response as to whether they want to start a game (y/n)
    char startResponse[32];
    fgets(startResponse, sizeof(startResponse), stdin);
    int validInput = 0;

    // make sure the input is valid (aka, only one letter, no numbers, and either y, n, Y, or N)
    while (validInput == 0) {
        if (strcmp(&startResponse[1], "\n") != 0) {
            puts("ERROR: Your input must be a single letter!");
            puts("Please enter a valid response: ");
            fgets(startResponse, sizeof(startResponse), stdin);
        } else if (startResponse[0] != 121 && startResponse[0] != 110 && startResponse[0] != 89 && startResponse[0] != 78) {
            puts("ERROR: Your input must be either Y or N!");
            puts("Please enter a valid response: ");
            fgets(startResponse, sizeof(startResponse), stdin);
        } else { validInput = 1; }
    }
    // if the user's input is a capital Y or N, make it lower case to send to server
    output = startResponse[0];
    if (output == 89 || output == 78) {
        output += 32;
    }

    // if the input is y, then send this to the server
    if (output == 'y') {
        uint8_t questionResponse[2];
        questionResponse[0] = (uint8_t) 1;
        questionResponse[1] = (uint8_t) output;

        // send message with y to the server
        numBytes = send(clientSock, questionResponse, sizeof(questionResponse), 0);
        if (numBytes < 0) {
            puts("Sending the account name failed...");
            close(clientSock);
            exit(1);
        }
    } else {
        // else close down the client connection to the server
        puts("User has indicated they do not wish to start a game.");
        puts("Shutting down connection to game server...");
        close(clientSock);
        exit(1);
    }

    // if yes, then we prepare to start the game on the client end
    char userGuess;
    char input[32];

    
    while (1) {
        memset(&rcvBuf, 0, RCVBUFSIZE);
        // receive the message from the server
        numBytes = recv(clientSock, rcvBuf, RCVBUFSIZE, 0);
        if (numBytes < 0) {
            puts("Receiving of the word transmission failed...");
            close(clientSock);
            exit(1);
        } else if (numBytes == 0) {
            puts("Receiving connection closed prematurely...");
            close(clientSock);
            exit(1);
        }

        // break the message down and determine whether it was a game-terminating
        // message (game over or server-overload) or if it was a game-continuing
        // message (info about word length, num incorrect guess, etc.)
        int msgFlag = (int)rcvBuf[0];


        // if the message flag is not zero, we know that it's a game-terminating message
        // and we therefore pull out the information and determine the message
        if (msgFlag != 0) {
            char message[msgFlag + 1];
            for (int i = 0; i < msgFlag; i++) {
                message[i] = (char)rcvBuf[i + 1];
            }
            message[msgFlag] = '\0';
            
            determineServerMessage(msgFlag, message);
            // we then close the client connection
            close(clientSock);
            break;
        } else {
            // otherwise we know it's got to be a game-continuing message
            // and we break it down to get the information necessary
            // to print out the information to the user and allow them
            // to continue playing

            int length = (int)rcvBuf[1];
            int incorrect = (int)rcvBuf[2];
            char word[length];
            char guesses[6];
            // pull out all of the encoded word leters
            for (int i = 0; i < length; i++) {
                word[i] = (char)rcvBuf[i + 3];
            }
            // if there are incorrect guesses, pull them out of the message
            if (incorrect != 0) {
                for (int i = 0; i < incorrect; i++) {
                    guesses[i] = (char)rcvBuf[i + 3 + length];
                }
            }

            fflush(stdin);
            // pass the parameters into the function to handle them and then print
            // out the appropriate message to the client
            handleWordCase(msgFlag, length, incorrect, word, guesses);
            printf("Please guess a letter: ");
            // get the user's letter input from the command line
            fgets(input, sizeof(input), stdin);
            int validInput = 0;

            // make sure that the input is a valid one (aka only one letter, no numbers and that
            // you haven't already guessed it)
            while (validInput == 0) {
                int alreadyGuessed = 0;
                for (int i = 0; i < numLettersGuessed; i++) {
                    if (input[0] == guessedLetters[i]) {
                        alreadyGuessed = 1;
                    }
                }
                // check if there are multiple letters entered in the command line
                if (strcmp(&input[1], "\n") != 0) {
                    puts("ERROR: Your input must be a single letter!");
                    puts("Please guess a letter: ");
                    fgets(input, sizeof(input), stdin);
                } else if (input[0] < 65 || input[0] > 122 || (input[0] > 90 && input[0] < 97)) {
                    // make sure that the input is specifically a letter
                    puts("ERROR: Your input must be a letter!");
                    puts("Please guess a letter: ");
                    fgets(input, sizeof(input), stdin);
                } else if (alreadyGuessed == 1) {
                    // make sure that the letter has not already been guessed
                    printf("ERROR: You have already guessed the letter '%c'!\n", input[0]);
                    puts("Please guess a letter: ");
                    fgets(input, sizeof(input), stdin);
                } else {
                    validInput = 1;
                }
            
            }
            userGuess = input[0];

            // if the letter entered is a capital letter, make it lower case to send to the server
            if (userGuess > 64 && userGuess < 91) {
                userGuess += 32;
            }
            
            // add the letter to the list of letters already guessed (to make sure we don't guess it again later)
            guessedLetters[numLettersGuessed] = userGuess;
            numLettersGuessed++;
            

            // create our two byte message with the letter in the second byte
            uint8_t userMessage[2];
            userMessage[0] = (uint8_t) 1;
            userMessage[1] = (uint8_t) userGuess;

            // send the message to the server as our guess
            numBytes = send(clientSock, userMessage, sizeof(userMessage), 0);
            if (numBytes < 0) {
                puts("Sending the account name failed...");
                close(clientSock);
                exit(1);
            }
        }
    }
}