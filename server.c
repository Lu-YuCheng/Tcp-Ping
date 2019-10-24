#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    // Input
    if(argc != 2) {
        printf("usage : ./server listen_port\n");
        exit(1);
    }
    int fd;
    int port = atoi(argv[1]);
    struct sockaddr_in srv;

    // create socket
    if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    // bind
    srv.sin_family = AF_INET;
    srv.sin_port = htons(port);
    srv.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(fd, (struct sockaddr*) &srv, sizeof(srv)) < 0) {
        perror("bind");
        exit(1);
    }
    
    // listen
    if(listen(fd, 5) < 0) {
        perror("listen");
        exit(0);
    }

    while(1) {
        // accept
        int newfd;
        struct sockaddr_in cli;
        int cli_len = sizeof(cli);
        newfd = accept(fd, (struct sockaddr*) &cli, (socklen_t*)&cli_len);
        if(newfd < 0) {
            perror("accept");
            exit(1);
        }

        // read and write
        int msg;
        recv(newfd, &msg, sizeof(int), 0);
        int number = msg;
        for(int i = 0; i < number || number == 0 ; i++) {
            if(recv(newfd, &msg, sizeof(int), 0) < 0) {
                perror("recv");
            }
            else {
                send(newfd, &msg, sizeof(int), 0);
                printf("recv from %s:%hu\n", inet_ntoa(cli.sin_addr),cli.sin_port);
            }
        }
        shutdown(newfd, SHUT_RDWR);
    }
    return 0;
}
