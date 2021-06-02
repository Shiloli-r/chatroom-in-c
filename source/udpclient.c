// Standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// For creating an endpoint for communication
#include <sys/types.h>
#include <sys/socket.h>

// htonl, htons, ntohl, ntohs - convert values between host and network byte order
#include <netinet/in.h>

#include <fcntl.h>  // for open
#include <unistd.h> // for close

int main()
{
    // Socket variable
    struct sockaddr_in server;

    // File descriptor. (Socket Id)
    int fd;

    // Connection file descriptor. (Connection Id)
   // int conn;

    // Message store for messages from server
    char message[256] = "";
    char new_message[256] = "";
    char response[256] = "Connection success";


    fd = socket(AF_INET, SOCK_DGRAM, 0);



    server.sin_family = AF_INET;
    server.sin_port = htons(9056);
    server.sin_addr.s_addr = INADDR_ANY;

    // Client connects to server
   // int connection_status = connect(fd, (struct sockaddr *)&server, sizeof(server)); //This connects the client to the server.
    
    // check for error with the connection
    if (!(fd > 0))
    {
        printf("There was an error making a connection to the remote socket \n\n");
        return -1;
    }
    int size = sizeof(server);


    while (1)
    {
        printf(" Client: Enter a message: \n");
        fgets(message, 100, stdin);
        sendto(fd, message, 256, 0,(struct sockaddr*)&server,sizeof(server));

        //send(fd, message, strlen(message), 0);
        recvfrom(fd, new_message, 256, 0,(struct sockaddr *)&server,&size);

        //recv(fd, new_message, sizeof(new_message), 0);
        printf("Response from Server: %s \n", new_message);
        //An extra breaking condition can be added here (to terminate the while loop)
    }

    close(fd);

    return 0;
}
