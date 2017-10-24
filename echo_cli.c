#include    "unp.h"

void
str_cli(FILE *fp, int sockfd)
{
    int         maxfdp1, stdineof;
    fd_set      rset;
    char        buf[MAXLINE];
    int     n;
    signal(SIGPIPE, SIG_IGN);
    char red[10] = "\033[31m\033[1m";
    char normal[10] = "\033[0m";

    stdineof = 0;
    FD_ZERO(&rset);
    for ( ; ; ) {
        if (stdineof == 0)
            FD_SET(fileno(fp), &rset);
        FD_SET(sockfd, &rset);
        maxfdp1 = max(fileno(fp), sockfd) + 1;
        Select(maxfdp1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(sockfd, &rset)) {  /* socket is readable */
            if ( (n = Read(sockfd, buf, MAXLINE)) == 0) {
                if (stdineof == 1)
                    return;     /* normal termination */
                else
                    err_quit("str_cli: server terminated prematurely");
            }
            Write(fileno(stdout), red, sizeof(red));
            Write(fileno(stdout), buf, n);
            Write(fileno(stdout), normal, sizeof(normal));
        }

        if (FD_ISSET(fileno(fp), &rset)) {
            if ( (n = Read(fileno(fp), buf, MAXLINE)) == 0) {
                stdineof = 1;
                Shutdown(sockfd, SHUT_WR);
                FD_CLR(fileno(fp), &rset);
                continue;
            }

            Writen(sockfd, buf, n);
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

        str_cli(stdin, sockfd);     /* do it all */
    exit(0);
}
