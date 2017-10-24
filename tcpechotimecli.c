#include    "unp.h"


void sig_chld(int signo){
    pid_t pid;
    int stat;
    while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        printf("Child %d of client terminated\n", pid);
    return;
}

int main(int argc, char **argv){
    int     pipe_id[2], nread, i;
    char    type[1024], str[INET_ADDRSTRLEN], **pptr, buf[1024], *hostaddr;
    pid_t   pid;
    struct  hostent *hptr;

    if (argc != 4)
        err_quit("usage: ipaddressORhostname echoport timeport");

    
    if(argv[1][0] > '9' || argv[1][0] < '0'){   //get host by name
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
        
        hostaddr = str;
    }
    
    else{                                   //get host by address
        in_addr_t addr = inet_addr(argv[1]);
        if((hptr = gethostbyaddr((const char *)&addr, 4, AF_INET)) == NULL)
            err_msg("gethostbyaddr error for host: %s:%s", argv[1], hstrerror(h_errno));
        hostaddr = argv[1];
        printf("\nthe server host of %s: ", argv[1]);
        printf("\t%s\n", hptr -> h_name);
    }
    //----------------------------------------------------------------------------------------------------
    // infinite loop in which it queries the user which service is being requested------------------------

    signal(SIGCHLD, sig_chld);


    for(;;){
        printf("Select which service do you want: echo, time, quit.\n");

        if(fgets(type, 1024, stdin) == NULL)
            perror("tcpechotimecli: fgets type error!");

        if (strncmp(type, "quit", 4) != 0 && strncmp(type, "time", 4) != 0 && strncmp(type, "echo", 4) != 0){
            continue;
        }

        if (pipe(pipe_id) == -1)
                perror("pipe failed");

        if(strncmp(type, "quit", 4) == 0){
            printf("Choose quit service\n");
            exit(0);
        }

        if ((pid = fork()) == 0){         //child process
            close(pipe_id[0]);
            dup2(pipe_id[1], 101);
            close(pipe_id[1]);
            if(strncmp(type, "echo", 4) == 0){
                printf("Choose echo service.\n");
                if((execlp("xterm", "xterm", "-e", "./echo_cli", hostaddr, argv[2], (char *) 0)) < 0){
                    err_quit("execlp fail");
                    exit(0);
                }
            }
            else if(strncmp(type, "time", 4) == 0){
                printf("Choose time service.\n");
                if((execlp("xterm", "xterm", "-e", "./time_cli", hostaddr, argv[3], (char *) 0)) < 0){
                    err_quit("execlp fail");
                    exit(0);
                }
            }
        }
        else{                  //parent process
            close(pipe_id[1]);
            dup2(pipe_id[0], 100);
            close(pipe_id[0]);
        }
        sleep(1);

    }
}




