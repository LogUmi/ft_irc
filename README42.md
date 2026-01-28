*This project was developed as part of the 42 curriculum. by lgerard, dbonali, and oliradet*

# ft_irc

## Overview

**ft_irc** is a simplified implementation of an **Internet Relay Chat
(IRC) server**, written in **C++98**.\
The objective of this project is to design a real-time network server
handling multiple clients simultaneously, while respecting strict
constraints on non-blocking I/O and system-level programming.

The server supports core IRC features such as user registration, channel
management, messaging, and operator privileges.\
Server-to-server communication is intentionally **not** implemented.

------------------------------------------------------------------------

## Features

-   Non-blocking TCP server using `poll()`
-   Multiple simultaneous client connections
-   User authentication and registration:
    -   `PASS`, `NICK`, `USER`
-   Channel management:
    -   `JOIN`, `PART`, `KICK`, `INVITE`, `TOPIC`
-   Channel modes:
    -   `i` (invite-only)
    -   `t` (topic protection)
    -   `k` (channel key)
    -   `o` (operator)
    -   `l` (user limit)
-   Messaging:
    -   `PRIVMSG`, `NOTICE`
-   Graceful disconnection handling:
    -   `QUIT`

------------------------------------------------------------------------

## Compilation

``` bash
make
```

This will generate the executable:

``` bash
ircserv
```

### Are also available with the Makefile:

Delete objects .o/.d:
``` bash
make clean
```  

Delete executable file and objects .o/.d:
``` bash
make fclean
``` 

Compile again after deletion of executable file and objects .o/.d:
``` bash
make re
``` 
------------------------------------------------------------------------

## Usage

``` bash
./ircserv <port> <password>
```

-   **port**: TCP port the server will listen on\
-   **password**: password required to authenticate clients /
				  type "" for password free

Example:

``` bash
./ircserv 6667 mypassword
```

------------------------------------------------------------------------

## Connecting to the Server

You can connect using a standard IRC client (e.g. **irssi**,
**WeeChat**, **HexChat**) or with `netcat` for testing:

``` bash
nc -C 127.0.0.1 6667
```

Registration sequence:

    PASS mypassword
    NICK mynickname
    USER myuser 0 * :Real Name

------------------------------------------------------------------------

## Technical Choices

-   Written in **C++98**
-   Compiled with `-Wall -Wextra -Werror`
-   Non-blocking sockets using `fcntl(O_NONBLOCK)`
-   Single event loop based on `poll()`
-   No multithreading and no `fork()`
-   Clear separation of responsibilities:
    -   **Server**: networking, poll loop, connection handling
    -   **User**: client state and I/O buffers
    -   **Channel**: channel state and members
    -   **Commands**: IRC command logic
    -   **Parser**: command parsing

------------------------------------------------------------------------

## Commands list

The commands in the following list include the mandatory implemented ones(*), as well as some additional commands required for the project (to connect with LimaChat for example):
- CAP
- INVITE*
- JOIN*
- KICK*
- MODE*
- NICK*
- NOTICE
- PART
- PASS*
- PRIVMSG*
- PING
- PONG
- QUIT*
- TOPIC*
- USER*

------------------------------------------------------------------------

## References

### IRC Protocol

-   RFC 1459 --- Internet Relay Chat Protocol
-   RFC 2812 --- Internet Relay Chat: Client Protocol
-   https://modern.ircdocs.horse/

### System Programming

-   `man poll`
-   `man socket`
-   `man recv`
-   `man send`
-   `man fcntl`

------------------------------------------------------------------------

## AI Usage Disclaimer

AI tools were used **only as an assistance** for: - Clarifying IRC
protocol behavior - Verifying numeric replies and error codes -
Reviewing logic and identifying edge cases

All code was written, reviewed, and fully understood by the authors.\
No AI-generated code was blindly copied.

------------------------------------------------------------------------

## Notes

This project was developed and tested on Unix-based systems using real
IRC clients.\
The implementation aims to match the behavior of a standard IRC server
for the supported features.
