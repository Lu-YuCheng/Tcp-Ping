#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

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

    // set up select parameter
    int max_fd = listen_fd;
    fd_set master, rfds;
    FD_ZERO(&master);
    FD_SET(listen_fd, &master);

    // var used in while
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    while(1) {
        rfds = master;
        select(max_fd+1, &rfds, NULL, NULL, NULL);
        for(int fd = 0; fd <= max_fd; fd++) {
            if(FD_ISSET(fd, &rfds)) {
                // accept new connection
                if(fd == listen_fd) {
                    int client_fd = accept(listen_fd, (struct sockaddr*) &client_addr, &addr_len);
                    if(client_fd < 0) {
                        perror("accept");
                        exit(1);
                    }
                    FD_SET(client_fd, &master);
                    if(client_fd > max_fd) {
                        max_fd = client_fd;
                    }
                }
                // read and write
                else {
                    int msg;
                    char buf[256];
                    int nbytes = recv(fd, &msg, sizeof(int), 0);
                    if(nbytes < 0) {
                        perror("recv");
                    }
                    else if(nbytes == 0) {
                        FD_CLR(fd, &master);
                        shutdown(fd, SHUT_RDWR);
                    }
                    else {
                        send(fd, &msg, sizeof(int), 0);
                        getpeername(fd, (struct sockaddr*) &client_addr, &addr_len);
                        printf("recv from %s:%hu\n", inet_ntop(AF_INET,&client_addr.sin_addr,buf,256),client_addr.sin_port);
                    }
                }
            }
        }
    }
    return 0;
}
