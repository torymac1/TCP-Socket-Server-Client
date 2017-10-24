#include "unp.h"

void
tim_cli_1(int sockfd){
    int n;
    char buf[MAXLINE+1];
    signal(SIGPIPE, SIG_IGN);

    while((n = read(sockfd, buf, MAXLINE)) > 0){
        buf[n] = 0;
        if(fputs(buf, stdout) == EOF)
            err_sys("time_cli: fputs error.");
    }
    if (n <= 0)
        err_sys("time_cli: read error.");
}

void tim_cli(int sockfd){

    ssize_t nread;
    char    buf[MAXLINE];

    signal(SIGPIPE, SIG_IGN);
    for(;;){
        if((nread = readline(sockfd, buf, MAXLINE)) <= 0){
            err_sys("time_cli: readline error!");
            break;
        }

        if(write(fileno(stdout), buf, strlen(buf)+1) <= 0){
            if(errno == EINTR){
                snprintf(buf, sizeof(buf), "\n***Client termination: socket read returned with value -1, errno = EINTR***\n");
            break;
            }
        }
    }
}

int main(int argc, char **argv){

    printf("echo_cli start");
    int     sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(atol(argv[2]));
        if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
            err_quit("inet_pton_loose error");

        if(connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
        err_sys("connect error");

        tim_cli(sockfd);     /* do it all */
    exit(0);
}