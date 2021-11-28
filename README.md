# chatroom-in-c

Tested on ubuntu 20.04

### Install dependencies
`sudo apt-get install build-essential`  
`sudo apt-get install net-tools`

### Directory structure
bin/: Final compiled binary files  
source/: C source code  
Makefile: automation tool

### Compiling the program
Open terminal in the chat directory and run:  
`make compile`

### Running the program(s)
Start the server first  
`./bin/server`

Start the client only after starting the server  
`./bin/client`

Note:
Both the client and server can only send a message once it has received one and vice versa.

To avoid subsequent connection error(s): close the client before closing the server.

### Net commands
Check if port is in use:  
`netstat -ltnp | grep -w ':9051'`

Kill process that's using it:  
`kill -9 <PID>`
