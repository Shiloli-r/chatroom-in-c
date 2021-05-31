#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 2048
#define PORT 9051

static _Atomic unsigned int cli_count = 0;
static int uid = 10;

/* Client structure */
typedef struct{
    struct sockaddr_in address;
    int sockfd;
    int uid;
    char name[32];
} client_t;

client_t *clients[MAX_CLIENTS];


//function to clear the screen
void str_overwrite_stdout() {
    printf("\r%s", "> ");
    fflush(stdout);
}

// function to trim input \n
void str_trim_lf (char* arr, int length) {
    int i;
    for (i = 0; i < length; i++) { // trim \n
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

//function to print ip addresses
void print_client_addr(struct sockaddr_in addr){
    printf("%d.%d.%d.%d",
           addr.sin_addr.s_addr & 0xff,
           (addr.sin_addr.s_addr & 0xff00) >> 8,    // binary right shift by 8 bits
           (addr.sin_addr.s_addr & 0xff0000) >> 16,
           (addr.sin_addr.s_addr & 0xff000000) >> 24);
}

/* Add clients to queue */
void enqueue(client_t *cl){

    for(int i=0; i < MAX_CLIENTS; ++i){
        if(!clients[i]){
            clients[i] = cl;
            break;
        }
    }

}

/* Remove clients to queue */
void dequeue(int uid){

    for(int i=0; i < MAX_CLIENTS; ++i){
        if(clients[i]){
            if(clients[i]->uid == uid){
                clients[i] = NULL;
                break;
            }
        }
    }

}

/* Send message to all clients except sender */
void send_message(char *msg, int uid, char* receiver){  // takes the msg to be sent, id of sender and receiver

    //IFF a receiver is specified (Send only to that receiver) forget everyone else
    if(receiver != NULL){
        for(int i=0; i<MAX_CLIENTS; ++i){
            if(clients[i]) {
                if (strcmp(clients[i]->name, receiver) == 0) { // if the client name matches the receiver name
                    if (write(clients[i]->sockfd, msg, strlen(msg)) < 0) {
                        perror("ERROR: write to descriptor failed");
                        break;
                    }else{
                        goto finish;
                    }
                }
            }
        }

    //The username does not exist in the chatroom
    for(int i=0; i<MAX_CLIENTS; ++i){
        if(clients[i] && clients[i]->uid == uid){
            bzero(msg, strlen(msg));
            strcpy(msg, "This user does not exist! \n");
            write(clients[i]->sockfd, msg, strlen(msg));
        }
    }

    }else{
        //If no receiver is specified, send to all
        for(int i=0; i<MAX_CLIENTS; ++i){
            if(clients[i]){
                if(clients[i]->uid != uid){   // if the client id is not sender's id, send message the client
                    if(write(clients[i]->sockfd, msg, strlen(msg)) < 0){
                        perror("ERROR: write to descriptor failed");
                        break;
                    }
                }
            }
        }

    }
    finish:;
}

/* Handle all communication with the client */
void *handleClient(void *arg){
    char buff_out[BUFFER_SIZE];
    char buff[BUFFER_SIZE], message[BUFFER_SIZE];
    char name[32];
    char receiver[32];
    int leave_flag = 0;

    cli_count++;
    client_t *cli = (client_t *)arg;

    // Name
    if(recv(cli->sockfd, name, 32, 0) <= 0 || strlen(name) <  2 || strlen(name) >= 32-1){
        printf("You did not enter your name.\n");
        leave_flag = 1;
    } else{
        // sending message and printing when a new client joins
        strcpy(cli->name, name);
        sprintf(buff_out, "%s has joined\n", cli->name);
        printf("%s", buff_out);
        send_message(buff_out, cli->uid, NULL);

        //SEND LIST OF CONNECTED CLIENTS
        for(int i=0; i<MAX_CLIENTS; i++){
            if(clients[i] && clients[i]->uid != cli->uid){
                bzero(buff_out, strlen(buff_out));
                sprintf(buff_out, "%s is in the Chatroom \n", clients[i]->name);
                send_message(buff_out, cli->uid, cli->name);
            }
        }

    }

    bzero(buff_out, BUFFER_SIZE);

    for(;;){
        if (leave_flag) {
            break;
        }

        int receive = recv(cli->sockfd, buff, BUFFER_SIZE, 0);
        if (receive > 0){
            if(strlen(buff) > 0){


                if(strncmp(buff, "@", 1) == 0){
                    //extract the recipient's name
                    bzero(receiver, sizeof(receiver));
                    for(int i=0; i< strlen(buff); ++i) {
                        if (buff[i] == ' ' || buff[i] == 10){  // new line character
                            str_trim_lf(buff, strlen(buff));
                            memcpy(message, buff + 1 + i, strlen(buff) + 1 - i);
                            break;
                        }
                        if(buff[i] == 64)  // The @ symbol
                            continue;
                        strcat(receiver, (char[]){buff[i], 0});
                    }

                    //Attach sender to message
                    sprintf(buff_out, "[%s]: %s \n", cli->name, message);
                    send_message(buff_out, cli->uid, receiver);
                }else{
                    str_trim_lf(buff, strlen(buff));
                    //Attach sender to message
                    sprintf(buff_out, "[%s]: %s \n", cli->name, buff);
                    send_message(buff_out, cli->uid, NULL);
                }

                str_trim_lf(buff_out, strlen(buff_out));
                printf("%s\n",  buff_out);
            }
        } else if (receive == 0 || strcmp(buff, "exit") == 0){  // an exit message for client exit
            sprintf(buff_out, "%s has left\n", cli->name);
            printf("%s", buff_out);
            send_message(buff_out, cli->uid, NULL);
            leave_flag = 1;
        } else {
            printf("ERROR: -1\n");
            leave_flag = 1;
        }

        bzero(buff_out, BUFFER_SIZE);
    }

    close(cli->sockfd);
    dequeue(cli->uid);
    free(cli);
    cli_count--;

    return NULL;
}

int main(){

    char *ip = "127.0.0.1";
    int option = 1;
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;



    /* Socket settings */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(PORT);
    /* Ignore pipe signals */
    signal(SIGPIPE, SIG_IGN);

    /*
     * The setsockpt function is used to set the socket options (define the socket behaviour)
     * The first argument is the name of the socket (listenfd)
     * int level: is set to SOL_SOCKET which sets options at the socket level
     * int option_name:
     *          SO_REUSEADDR -  enables local address reuse
     *                          Indicates that the socket can be reused. If set before binding the socket,
     *                          ensures successful binding, unless there is another socket with
     *                          EXACTLY the same port and address combination..(ensures No Address already in use error)
     *          SO_REUSEPORT - enables duplicate ports and bindings
     *                          allows binding an arbitrary number of sockets
     *                          to exactly the same source address and port provided all prior bound sockets
     *                          also had SO_REUSEPORT set before being bound
     *
     * char * optval:  A pointer to buffer containing data need by the set command.
     *                 When the integer is nonzero, the option is enabled.
     *
     * int optlen: size of data pointed to by optval (in our case, it is size of int)
     * */
    if(setsockopt(listenfd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR), (char*)&option, sizeof(option)) < 0){
        perror("ERROR: setting socking options failed");
        return EXIT_FAILURE;
    }

    /* Bind */
    if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR: Socket binding failed");
        return EXIT_FAILURE;
    }

    /* Listen */
    if (listen(listenfd, 10) < 0) {
        perror("ERROR: Socket listening failed");
        return EXIT_FAILURE;
    }

    // Create two sets of file descriptors
    fd_set current_sockets, ready_sockets;

    // initialize the set of current sockets
    FD_ZERO(&current_sockets);

    //Add the server socket to list of current sockets
    FD_SET(listenfd, &current_sockets);


    printf("=== CHATROOM IS OPEN ===\n");

    for(;;){ //Server listens eternally

        //copy the current sockets to the ready sockets (select is 'destructive')
        ready_sockets = current_sockets;

        //Calling select
        /*
         * FD_SETSIZE - The max number of file descriptors
         *  arg1 - set of fds to check for reading
         *  arg2 - set of fds to check for writing
         *  arg3 - set of fds to check for errors
         *  arg4 - optional timeout value (check for how long?)
         * */
        if(select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0){
            perror("Select Error");
            exit(EXIT_FAILURE);
        }
        /*
         * NOTE: We pass into select FDs to be watched, but when select returns, that fd set contains only those ready
         *
         * */

        socklen_t clilen = sizeof(cli_addr); // size of the cli_addr

        for(int i=0; i<FD_SETSIZE; i++){  // Cycle through the range of all possible FDs
            if(FD_ISSET(i, &ready_sockets)){ // Check if each FD is set, If set, there is data to be read from the FD
                if(i == listenfd){   // if fd is the server socket, Then it is a NEW CONNECTION
                    connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen); // accept the new connection
                    FD_SET(connfd, &current_sockets); // Add that client to list of sockets to watch
                }else{ // Means that connection is a client, so you handle the client
                    /* Client settings */
                    client_t *cli = (client_t *)malloc(sizeof(client_t));
                    cli->address = cli_addr;
                    cli->sockfd = connfd;
                    cli->uid = uid++;

                    enqueue(cli);
                    handleClient((void*)cli);  // handle the client
                    FD_CLR(i, &current_sockets); // Remove the socket from the list of FDs being watched
                }
            }
        }

    }

    return EXIT_SUCCESS;
}