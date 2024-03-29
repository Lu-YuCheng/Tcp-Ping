#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

int retval[2] = {0,1};
int client_number = 0;
void cleanup_handler(void *arg);
void *client_handler(void *arg);
typedef struct
{
    int fd;
    struct sockaddr_in addr;
    socklen_t addr_len;
}Client_info;

int main(int argc, char **argv)
{
    // Input
    if(argc != 2) {
        printf("usage : ./server listen_port\n");
        exit(1);
    }
    int listen_fd;
    int port = atoi(argv[1]);
    struct sockaddr_in srv;

    // create socket
    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    // bind
    srv.sin_family = AF_INET;
    srv.sin_port = htons(port);
    srv.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(listen_fd, (struct sockaddr*) &srv, sizeof(srv)) < 0) {
        perror("bind");
        exit(1);
    }
    
    // listen
    if(listen(listen_fd, 5) < 0) {
        perror("listen");
        exit(0);
    }

    while(1) {
        if(client_number > 5)
            sleep(1);
        Client_info *client = (Client_info *)malloc(sizeof(Client_info));
        pthread_t thread_id;
        client->fd = accept(listen_fd, (struct sockaddr*) &(client->addr), &(client->addr_len));
        pthread_create(&thread_id, NULL, client_handler, client);
    }
    return 0;
}
void *client_handler(void *arg) {
    // set up cleanup fnt 
    pthread_cleanup_push(cleanup_handler, (void *)arg);
    client_number++;

    Client_info *client = (Client_info *)arg;
    client->addr_len = sizeof(client->addr);
    if(client->fd < 0) {
        perror("accept");
        pthread_exit(&retval);
    }

    // read and write
    int msg;
    char ip[32];
    int nbytes; 
    int flag = 1;

    while(flag) {
        nbytes = recv(client->fd, &msg, sizeof(int), 0);
        if(nbytes < 0) {
            perror("recv");
            pthread_exit(&(retval[1]));
        }
        else if(nbytes == 0) {
            shutdown(client->fd, SHUT_RDWR);
            flag = 0;
        }
        else {
            send(client->fd, &msg, sizeof(int), 0);
            getpeername(client->fd, (struct sockaddr*) &(client->addr), &(client->addr_len));
            printf("recv from %s:%hu\n", inet_ntop(AF_INET,&(client->addr.sin_addr),ip,32),client->addr.sin_port);
        }
    }

    // call cleanup fnt
    pthread_cleanup_pop(1);

    pthread_exit(&(retval[0]));
}
void cleanup_handler(void *arg) {
    Client_info *client = (Client_info *)arg;
    free(client);
    client_number--;
}
