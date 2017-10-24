#include "unp.h"

void get_sigpipe(int n){
    printf("client get signal SIGPIPE %d\n",n);
}

void get_echo_srv(int sockfd){
    int n, max_fd_p;
    char buf[MAXLINE + 1];
    fd_set rset;

    FD_ZERO(&rset);
    for( ; ; ) {
        FD_SET(sockfd, &rset);

        max_fd_p = sockfd + 1;
        if((n = select(max_fd_p, &rset, NULL, NULL, NULL)) < 0) {
            printf("Client termination: EPIPE error detected or ctrl-c pressed.\n");
        }

        if(FD_ISSET(sockfd, &rset)) {
            if((n = readline(sockfd, buf, MAXLINE)) == 0) {
                printf("echo client termination: socket read returned with value 0\n");
                return ;
            }
            if(n < 0) {
                printf("echo client termination: socket read returned with value %d\n", n);
                err_sys("read error");
            }
            buf[n] = 0;
            if(write(sockfd, buf, n) < 0) {
                err_sys("write error");
            }
            //printf("received from client: %s", buf);
        }
    }
}

void get_echo_srv_1(int sockfd){
    int read_cnt;
    char buf[MAXLINE];
    static const char * CSI_Red = "\32[";
    static const char * CSI_Green = "\33[";
    for(;;){
        if((read_cnt = readline(sockfd, buf, MAXLINE)) == 0){
            printf("str_echo_srv: readline return 0!");
        }
        else if (read_cnt > 0){
            if (writen(sockfd, buf, read_cnt) <= 0){
                perror ("str_echo_srv: echo write to sockfd failed");
            }
        }
        else if (read_cnt < 0){
            if (errno == EINTR){
                perror("str_echo_srv: erron = EINTR");
                continue;
            }
            else
                err_sys("str_echo_srv: readline < 0");
        }
    }
}


void *echo_srv(void* p){

    int connfd_echo;
    connfd_echo = *((int *)p);
    signal(SIGPIPE, get_sigpipe);
    Pthread_detach(pthread_self());

    get_echo_srv(connfd_echo);
    Close(connfd_echo);
}

void get_time_srv(int sockfd){
    int n, max_fd_p;
    char buf[MAXLINE+1];
    fd_set rset;
    struct timeval time_val; /* for sleeping */
    time_t cur_time;

    time_val.tv_sec = 5;
    time_val.tv_usec = 0;

    FD_ZERO(&rset);
    for( ; ; ) {
        time_val.tv_sec = 5;
        time_val.tv_usec = 0;

        FD_SET(sockfd, &rset);
        cur_time = time(NULL);

        snprintf(buf, sizeof(buf), "%.24s\n", ctime(&cur_time));
        int buf_len = strlen(buf);
        if(write(sockfd, buf, buf_len) != buf_len) {
            err_sys("socket write error");
        }
        max_fd_p = sockfd + 1;
        if((n = select(max_fd_p, &rset, NULL, NULL, &time_val)) > 0) {
            
            printf("time client termination: EPIPE error detected or ctrl-c pressed.\n");
            return ;
        } 
    }
}

void *time_srv(void* p){

    int connfd_time;
    connfd_time = *((int *)p);
    signal(SIGPIPE, get_sigpipe);
    Pthread_detach(pthread_self());

    get_time_srv(connfd_time);
    Close(connfd_time);
}



int main(int argc, char **argv){
    int listen_echo, listen_time, *connfd_echo, *connfd_time, maxfdp1 ,i;
    pthread_t tid;
    socklen_t addrlen, len;
    fd_set rset;
    struct sockaddr *cliaddr;
    struct  hostent *hptr;
    char **pptr, str[INET_ADDRSTRLEN];
    

    if (argc == 4){
        if(argv[1][0] > '9' || argv[1][0] < '0'){   //get host by name
            printf("get host by name");
            if((hptr = gethostbyname(argv[1])) == NULL){
                err_msg("gethostbyname error for host: %s:%s", argv[1], hstrerror(h_errno));
            }

            switch(hptr -> h_addrtype){    
            case AF_INET:
                pptr = hptr -> h_addr_list;
                printf("\nthe IP address of %s: ", hptr -> h_name);
                inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str));
                printf("\t%s\n", str);
                break;
            default:
                err_ret("unknown address type");
                break;
            }
            char hostaddr[sizeof(str)];
            for(i = 0; i < sizeof(str); i++)
                    hostaddr[i] = str[i];
            printf("hostaddr: %s\n", hostaddr);
            listen_echo = tcp_listen(str, argv[2], &addrlen);
            listen_time = tcp_listen(str, argv[3], &addrlen);
        }
    
        else{                                   //get host by address
            in_addr_t addr = inet_addr(argv[1]);
            if((hptr = gethostbyaddr((const char *)&addr, 4, AF_INET)) == NULL)
                err_msg("gethostbyaddr error for host: %s:%s", argv[1], hstrerror(h_errno));
            printf("\nthe server host of %s: ", argv[1]);
            printf("\t%s\n", hptr -> h_name);
            listen_echo = tcp_listen(argv[1], argv[2], &addrlen);
            listen_time = tcp_listen(argv[1], argv[3], &addrlen);
        }
        
    }
    else
        err_quit ("usage: server ipaddressORhostname echoport timeport");

    cliaddr = Malloc(sizeof(int));
    connfd_echo = Malloc(sizeof(int));
    connfd_time = Malloc(sizeof(int));
    FD_ZERO(&rset);
    for(;;){
        len = addrlen;
        FD_SET(listen_echo, &rset);
        FD_SET(listen_time, &rset);
        maxfdp1 = max(listen_echo, listen_time) + 1;
        select(maxfdp1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(listen_echo, &rset)){
            if ((*connfd_echo = accept(listen_echo, cliaddr, &len)) < 0)
                perror("connfd_echo accpet error!");
            else{
                pthread_create(&tid, NULL, &echo_srv, connfd_echo);
            }
        }
        if (FD_ISSET(listen_time, &rset)){
            if ((*connfd_time = accept(listen_time, cliaddr, &len)) < 0)
                perror("connfd_time accpet error!");
            else{
                pthread_create(&tid, NULL, &time_srv, connfd_time);
            }
        }
    }
    free(cliaddr);
    free(connfd_echo);
    free(connfd_time);
    exit(0);
}