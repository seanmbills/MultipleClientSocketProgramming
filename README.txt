CS 3251 Project 2 -- Multiple Client-Server Connections to
                 Play multiple Concurrent Games of Hangman

Group Members: Sean Bills and Cameron Grey Huynh

High Level Explanation: The overall method utilized here was to create a server
that, upon a client connection attempting to be established, would either 
allow for a successful connection or return an "ERROR" of some sort to the user
alerting them that they could not connect to the server. This "ERROR" came in the
form of a message alerting the potential new Client connection that the server
is currently full (aka a "server-overload"). If, however, the Server was not full
and the Client could successfully connect to it, then the server would handle
creating a new thread in which that Client's game of Hangman could run. Thus,
ultimately, the game functions using Multithreading, allowing each of the three
concurrent games to run in their own thread within the main process. By doing this
it allows us to more easily not only terminate threads when the game itself has
run its course, but also keep track of the number of current connections via use
of a global variable that all of the threads have access to. When a Client is
successfully connected to the Server, the Client plays its own game of Hangman,
getting 6 incorrect guesses in which they can guess the correct word.

If the server is established with no defined dictionary passed in to the terminal
upon startup, then a default dictionary (stored within this directory as
"default.txt") will be used in order to choose the words for the Clients to guess.
If, however, a dictionary name is entered at the time the Server is started in 
the terminal, then that will be the dictionary from which all Client words will
be chosen.

Division of Work:
    Sean:
        -- created Client code
        -- added methods in Server code to handle threading
            and lookup words in the dictionary
        -- added file reading mechanism to Server code in 
            order to support dictionaries entered in to
            command line
    Grey:
        -- created Server code
        -- added methods in Client code to handle printing out
            Server responses and
        -- added checks in Client code for instances where user
            inputs either numbers or multiple letters
            simultaneously

How To Use:
    Required Dependencies/Packages: There should be no required packages or
        dependencies necessary to download in order to successfully run
        this program.

    Code Startup:
        All files contained within the directory should be up-to-date, but if
            a concern that this isn't the case exists, change directory into
            the directory in terminal (or command line) and type:
                make
            This will compile the submitted server and client code to their
            most recent build. Next, open a new terminal/command line and start
            up the Server code by typing the following:

                ./server [port Number] (optional) [dictionary name]

                ***NOTE: [port Number] -- here should be replaced with an actual
                    number of the port which you wish to establish your Server
                    on
                ***NOTE: (optional) [dictionary name] -- here represents an optional
                    command line argument representing the name of a text file containing
                    the dictionary from which you want your Server to choose its
                    words to use in its games of Hangman with various Clients; this
                    argument is not necessary, as the Server will instead use a
                    default dictionary called "default.txt" if no dictionary name
                    is entered
                ***NOTE: please note that all dictionary names must include the
                    appropriate ".txt" extension when entered in to the server
                    command line arguments; for example, if I wished to start the
                    server with a dictionary called "dictionary.txt" on port 5000
                    I would do so by typing:
                        ./server 5000 dictionary.txt
                    where the ".txt" extension is crucial

            Once the Server is started in its own terminal/command line, you can now
                begin to connect Clients to the Server and play games of Hangman via
                the Clients. In order to do this, open a new terminal/command line
                and change directory into the directory containing the project. Once
                inside the appropriate directory, type the following command to
                begin a Client in this terminal:

                    ./client [IP address] [port Number]

                ***NOTE: the port number should be the same as the one on which the
                    Server is running, or else the Client will not be able to
                    successfully connect to the Server and play the game

            If desired, up to 3 Clients can be connected to the same Server at once
                but if you attempt to connect a fourth Client to the same Server
                (prior to one of those Clients closing down/finishing their games)
                then the Server will respond with a "server-overload" message to the
                Client, causing that connection to be gracefully terminated.

            If, however, you're playing the game of Hangman on one of the Clients
                already connected to the Server, then the following controls apply:
                1) the Server will send a "Ready to start? (y/n)" message to the Client.
                    This message will be displayed on the Client's screen. You must
                    enter either an uppercase or lowercase Y or N in order to 
                    send a valid response to the Server. 
                    ***NOTE: if you attempt to enter a letter (or number) that is not
                        either an upper or lowercase Y or N, the Client will prompt
                        you to enter a valid response
                    ***NOTE: if you enter in an N, then the Client/Server connection
                        will promptly shut down and the Client will notify you of this
                        action
                2) if the Client enters a Y (indicating that yes they wish to begin the game)
                    then the Server will next access the appropriate dictionary and choose
                    a word for the Client to play a game of Hangman with
                        -- this word will then be sent to the Client as a series of "_"
                            indicating the number of letters that the word is long
                        -- the number of incorrect guesses remaining for the Client
                            to make will also be transmitted
                        -- the incorrect guesses will also be sent (at the start these are None)
                3) the client will then have all of this information displayed to it
                    and will be prompted to enter the letter which they wish to guess.
                        -- if the Client enters more than one character (or a number) then
                            they will be notified that they have done as much and will be
                            asked to input a valid one-character response until such a time
                            as they have successfully submitted a one-character response
                        -- if the Client has already guessed the letter they're attempting
                            to guess, they will be notified and prompted for a new letter
                4) Once the Client has submitted a valid letter, their guess will be sent to
                    the Server where it will be compared against the word for them to guess
                        -- if the letter is contained in the word, then the message sent to
                            the Client will be updated to include this letter in the place
                            of the "_" that was previously there
                        -- if the letter is not contained within the word, then the letter will
                            be added to the list of incorrect guesses made and will be sent to
                            the Client as such
                5) once the server has processed the Client's guess, it will send back a message
                    with the a flag, the word length, and the number of incorrect guesses, as well
                    as a string representing the word and a string representing the list of
                    incorrect guesses
                6) the Client will then break this information down and present it to the user
                    in the format mentioned above
                7) the program will then repeat steps 3-5 until such a time as the Client has
                    made 6 incorrect guesses for the letters contained within the word OR the
                    Client has successfully guessed all of the letters in the word and has,
                    therefore, won the game
                8) at such a time as the Client has guessed incorrectly 6 times or guessed all
                    letters correctly, then the appropriate message will be returned to the user
                    (either "You Win!" or "Game Over")
                9) this message will then be broken down by the Client and displayed to them
                10) once the game has ended (either Client won or lost), the connection to the
                    server will be terminated, freeing up a space for a new connection to be
                    made to the Server (thus starting a new game)

Test Results/Outputs:

    The Default Dictionary used (when one is not provided in the startup of the Server) is the
        following:
            letter
            test
            jazz
            trough
            music
            sweats
            broke
            shirt
            sings
            junk
            finger
            quiz
            major
            jacket
            hand

        It consists of 5 four-letter words, 5 five-letter words, anf 5 six-letter words.

    In the case of a successful game where the Client correctly guesses the word from the Server:
        Client Output:
            ./clientNew 127.0.0.1 5000                      # connect to the server
            Ready to start? (y/n)                           # print out the server's message asking if ready to start
            y                                               # user inputs response as y
            The word is: _ _ _ _                            # prints out the "encoded" word
            Number of incorrect guesses remaining: 6        # prints out the number of incorrect guesses the user has left
            Incorrect guesses so far: None                  # prints out the list of incorrect guesses (None if none guessed so far)
            Please guess a letter: h                        # user guesses a letter
            The word is: h _ _ _                            # letter is located in the word so server responds back with slightly decoded message
            Number of incorrect guesses remaining: 6        
            Incorrect guesses so far: None
            Please guess a letter: a                        # letter A is located in word so server decodes message further
            The word is: h a _ _ 
            Number of incorrect guesses remaining: 6
            Incorrect guesses so far: None
            Please guess a letter: n                        # letter N is located in word
            The word is: h a n _ 
            Number of incorrect guesses remaining: 6
            Incorrect guesses so far: None
            Please guess a letter: d                        # letter D is located in word
            Congratulations! You Win!                       # client prints the server's successful win message
            Game Over!


        Server Output:
            Creating new TCP socket...                                          ----|
            Binding the socket to the local address structure...                    |--- these three are only printed out on the startup of the server
            Listening on the socket...                                          ----|
            Accepting a new connection...                                       # server accepts the connection
            Creating a new thread for this new connection to run in...          # new thread is created for this Hangman game
            New Connection! Num Connections: 1                                  # server prints out number of connections currently running
            Sending the welcome 'Ready to start?' message to Client 4...        # sends message asking ready to start
            Received a response from Client 4...                                # receives client response (y/n)
            Client has indicated they want to play a game...                    # if client says y, start the game
            No dictionary name was given...                                     # determines whether a dictionary name was given as terminal argument
            Defaulting to the default dictionary...                             # no dictionary was provided so we default to default
            Finding a word...                                                   # server finds a word in the dictionary
            Sending the message to Client 4...                                  # sends the partially decoded message to the Client
            Sending the message to Client 4...
            Sending the message to Client 4...
            Sending the message to Client 4...
            Client 4 has won their game!                                        # Client has won, so server prints out that fact
            Closing connection with Client 4...                                 # closes connection with client
            Number of open connections: 0                                       # prints updated number of open connections
            Closing the thread in which this Hangman game is running...         # thread in which the game was running terminates

    In the case of an UNsuccessful game, where the Client fails to guess the word from the Server:
        Client Output:
            ./clientNew 127.0.0.1 5000
            Ready to start? (y/n)
            y
            The word is: _ _ _ _ 
            Number of incorrect guesses remaining: 6
            Incorrect guesses so far: None
            Please guess a letter: a                        # user guesses letter A
            The word is: _ _ _ _ 
            Number of incorrect guesses remaining: 5        # the guess was incorrect, therefore number of remaining incorrects decrements
            Incorrect guesses so far: A,                    # A gets added to the list of incorrect guesses
            Please guess a letter: i                        # user guesses letter I
            The word is: _ _ _ _ 
            Number of incorrect guesses remaining: 4        # incorrect guess, number of incorrect guesses decrements
            Incorrect guesses so far: A, I,                 # I gets added to the list of incorrect guesses
            Please guess a letter: e                        # user guesses E
            The word is: _ e _ _                            # the letter E was a correct guess, word gets updated
            Number of incorrect guesses remaining: 4        
            Incorrect guesses so far: A, I, 
            Please guess a letter: o                        # user guesses letter O incorrectly
            The word is: _ e _ _ 
            Number of incorrect guesses remaining: 3        # incorrect guesses remaining is updated
            Incorrect guesses so far: A, I, O,              # letter O gets added to list of incorrect letters
            Please guess a letter: u                        # user incorrectly guesses letter U
            The word is: _ e _ _ 
            Number of incorrect guesses remaining: 2        # number of incorrect guesses left is updated
            Incorrect guesses so far: A, I, O, U,           # U is added to list of incorrect guesses
            Please guess a letter: h                        # user incorrectly guesses letter H
            The word is: _ e _ _ 
            Number of incorrect guesses remaining: 1        # number of incorrect guesses left is updated
            Incorrect guesses so far: A, I, O, U, H,        # letter H is added to list of incorrect guesses
            Please guess a letter: g                        # user guesses letter G
            You lose :( Better luck next time!              # user has made 6 incorrect guesses therefore game ends
            Game Over!                                      # and user loses the game

        Server Output:
            Creating a new thread for this new connection to run in...      # a thread is created for this Hangman game to play in
            New Connection! Num Connections: 1                              # the number of connections is incremented with the new connection
            Sending the welcome 'Ready to start?' message to Client 7...    # the Server sends the Ready to Start message
            Received a response from Client 7...
            Client has indicated they want to play a game...
            No dictionary name was given...
            Defaulting to the default dictionary...                         # the default dictionary is used
            Finding a word...
            Sending the message to Client 7...
            Sending the message to Client 7...
            Sending the message to Client 7...
            Sending the message to Client 7...
            Sending the message to Client 7...
            Sending the message to Client 7...
            Sending the message to Client 7...
            Client 7 has lost their game...                                 # the Client has lost the game so the Server prints this out
            Closing connection with Client 7...                             # the connection with the client is closed
            Number of open connections: 0                                   # the number of open connections is decremented
            Closing the thread in which this Hangman game is running...     # the thread the game is running in closes

    In the case where the server is full and a server-overload message must be sent to the Client:
        Client Output:
            ./clientNew 127.0.0.1 5000                                  # client starts a connection to Server
            server-overload                                             # the Server responds with server-overload message and Client prints it
            The server is currently full. Please try again later.       # the Server is full

        Server Output:
            Accepting a new connection...                               # Server attempts to establish a new connection
            server-overload                                             # the Server has been overloaded so it prints this fact out
            The server is currently full and has alerted the new attempted connection of this fact...
            Closing connection with Client 13...                        # connection with the Client is closed
            Number of open connections: 3                               # number of open connections is printed out after this one is closed

