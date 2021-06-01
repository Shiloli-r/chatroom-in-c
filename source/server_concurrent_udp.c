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

//For creating threads
#include <pthread.h>

#define PORT 9056  // The port

struct sockaddr_in client; //destination address
int size = sizeof(client); //size of the client

// Message store for messages from server
char response[256] = "Connection success";
char new_message[256] = "";
char message[256] = "";

// Client structure
struct client
{
    char name[256];
    int sid;
    bool online;
};

//structure for arguments to the thread
typedef struct Args{
    int fd;
    struct sockaddr_in client;
} Args;


// Function to print errors and exit
void error(const char *msg){
    perror(msg);
    exit(1);
}


void* handleClient(void* p_fd){
    //type cast the fd to an integer pointer
    int fd = *((int *)p_fd);
    free(p_fd); //free the pointer

    printf("Server: Enter a message: ");
    fgets(new_message, 100, stdin);
    sendto(fd, new_message, 256, 0,(struct sockaddr*)&client,sizeof(client));

    return NULL;
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

        pthread_t thread;

        //receive message from client
        recvfrom(fd, message, 256, 0, (struct sockaddr*)&client, &size);
        printf("Response from client: %s  \n", message);

        //Arguments to the thread  (fd)
        int* p_fd = malloc(sizeof(int));
        *p_fd = fd;
        pthread_create(&thread, NULL, handleClient, p_fd);
    }

    // Close socket
    close(fd);

    return 0;
}