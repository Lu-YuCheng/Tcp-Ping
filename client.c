#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>

int main(int argc, char **argv)
{
    // parse the input
    int number = 0;
    struct timeval timeout = {1,0};
    long timeout_msec = 1000;
    int opt;
    while((opt = getopt(argc,argv, "n:t:")) != -1) {
        switch(opt) {
            case 'n':
                number = atoi(optarg);
                break;
            case 't':
                timeout_msec = atol(optarg);
                timeout.tv_sec = atol(optarg) / 1000;
                timeout.tv_usec = 1000 * (atol(optarg) % 1000);
                break;
            default:
                fprintf(stderr, "Usage: %s [-n number] [-t timeout] host:port\n", argv[0]);
                exit(1);
        }
    }
    
    if(optind >= argc) {
        fprintf(stderr, "Usage: %s [-n number] [-t timeout] host:port\n", argv[0]);
        exit(1);
    }

    // set up connection with each server
    int number_server = argc - optind;
    const char delim = ':';
    char **host = (char **)malloc(sizeof(char *) * number_server);
    char **port = (char **)malloc(sizeof(char *) * number_server);
    struct sockaddr_in *srv = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in) * number_server);
    int *fd = (int *)malloc(sizeof(int) * number_server);
    for(int i = 0; i < number_server; i++) {
        host[i] = strtok(argv[i+optind], &delim);
        port[i] = strtok(NULL, &delim);
        struct hostent *host_entity;
        if((host_entity = gethostbyname(host[i])) == NULL) {
            fprintf(stderr, "Host not found\n");
            exit(1);
        }
        srv[i].sin_family = host_entity->h_addrtype;
        srv[i].sin_port = htons(atoi(port[i]));
        srv[i].sin_addr.s_addr = *(long*)host_entity->h_addr;
        
        // create socket
        if((fd[i] = socket(AF_INET,SOCK_STREAM,0)) < 0) {
            perror("socket");
            exit(1);
        }

        // connect to server
        if(connect(fd[i], (struct sockaddr*) &srv[i], sizeof(srv[i])) < 0) {
            perror("connect");
            exit(1);
        }

        // set socket option
        setsockopt(fd[i], SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    }

    // ping
    struct timespec start, end;
    int buf;
    for(int msg = 0; msg < number || number == 0; msg++) {
        long double rtt_msec = 0;
        for(int i = 0; i < number_server; i++) {

            //send and recv msg
            clock_gettime(CLOCK_MONOTONIC, &start);

            send(fd[i], &msg, sizeof(msg), 0);
            while(recv(fd[i], &buf, sizeof(msg), 0) >= 0 && buf < msg) {}

            clock_gettime(CLOCK_MONOTONIC, &end);

            // compute rtt
            double timeElapsed = ((double)(end.tv_nsec - start.tv_nsec))/1000000.0;
            rtt_msec = (end.tv_sec-start.tv_sec) * 1000.0 + timeElapsed; 

            // output
            if( rtt_msec <= (long double)timeout_msec) {
                printf("recv from %s:%hu,RTT = %Lf msec\n", inet_ntoa(srv[i].sin_addr),ntohs(srv[i].sin_port),rtt_msec);
            }
            else {
                printf("timeout when connect to %s:%hu\n", inet_ntoa(srv[i].sin_addr),ntohs(srv[i].sin_port));
            }
        }
    }
    for(int i = 0; i < number_server; i++) {
        close(fd[i]);
    }

    return 0;
}
