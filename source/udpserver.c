// Standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// For creating an endpoint for communication
#include <sys/types.h>
#include <sys/socket.h>

// htonl, htons, ntohl, ntohs: convert values between host and network byte order.
// AF_INET (Adress Family)
#include <netinet/in.h>

#include <fcntl.h>  // for open
#include <unistd.h> // for close

#define PORT 9056  // The port

// Client structure
struct client
{
    char name[256];
    int sid;
    bool online;
};


// Function to print errors and exit
void error(const char *msg){
    perror(msg);
    exit(1);
}


void handleClient(int fd){
    struct sockaddr_in client;
    int size = sizeof(client);

    // Message store for messages from server
    char response[256] = "Connection success";
    char new_message[256] = "";
    char message[256] = "";
    recvfrom(fd, message, 256, 0, (struct sockaddr*)&client, &size);
    printf("Response from client: %s  \n", message);
    printf("Server: Enter a message: ");
    fgets(new_message, 100, stdin);
    sendto(fd, new_message, 256, 0,(struct sockaddr*)&client,sizeof(client));
}

int main()
{

    // Socket variable
    struct sockaddr_in server;

    // File descriptor. (Socket Id)
    int fd;

    // Create socket and return Socket Id
    fd = socket(AF_INET, SOCK_DGRAM, 0);

    bool connected[2] = {false, false};

    server.sin_family = AF_INET;         // Address family. IPv4 Internet protocols
    server.sin_port = htons(PORT);       // Listener port
    server.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0


    // Check for errors
    if (fd < 0)
        error("Connection failed. Error occurred \n");


    // Assigns identifier to the socket, prints an error if the binding fails
    if(bind(fd, (struct sockaddr *)&server, sizeof(server)) < 0)
        error("Socket Binding Failed");
    else{
        printf("[Server]: Server starting....\n");
        printf("[Server] listening at port %d \n", PORT);
    }


    while (1){  // The server listens eternally
        handleClient(fd);   // handles the client connection
    }

     // Close socket
    close(fd);

    return 0;
}