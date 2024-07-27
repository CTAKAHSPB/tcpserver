#include <stdlib.h> 
#include <stdio.h> 
#include <stdarg.h> 
#include <netdb.h> 
#include <pthread.h>
#include <string.h> 
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>

#define PORT 8080
#define THREAD_POOL_SIZE 5
#define MSG_BUFFER_SIZE 80 
#define POLLSIZE 1024
#define MAX_LOG_RECORD_SIZE 100

// server socker and epoll fds
int sockfd, epollfd;
// array of epoll events
struct epoll_event event[POLLSIZE];

// log function
void _log(const char *format, ...)
{
    va_list(args);

    time_t temp = time(NULL);
    struct tm *timeptr = localtime(&temp);

    char datetime[100];
    strftime(datetime, sizeof(datetime), "[%A %b %d] %r", timeptr);

    va_start(args, format);
    char log_record[MAX_LOG_RECORD_SIZE];
    vsnprintf(log_record, MAX_LOG_RECORD_SIZE, format, args);

    printf("%s: %s\n", datetime, log_record);
}

// function for chatting between client and server. 
void handle_connection(int connfd)
{ 
    char buff[MSG_BUFFER_SIZE]; 
    bzero(buff, MSG_BUFFER_SIZE); 

    // read the message from client and copy it in buffer 
    read(connfd, buff, sizeof(buff));
    // remove newline
    buff[strlen(buff) - 1] = '\0';
    // print buffer which contains the client contents 
    _log("From client: %s", buff); 
    // and send it back to  client 
    write(connfd, buff, sizeof(buff)); 
}

// worker thread function
void* thread_func(void *arg)
{
    while(1)
    {
        // get a number of events ready for handling
        int nready = epoll_wait(epollfd, event, POLLSIZE, -1);
        if(nready < 0)
        {
            _log("epoll_wait error");
            exit(1);
        }
        for(int i = 0; i < nready; i++)
        {
            if(event[i].data.fd == sockfd)
            {
                // new client connection
                if(event[i].events & EPOLLERR || event[i].events & EPOLLHUP) {
                    _log("server socket error");
                    exit(1);
                }

                // accept a connection
                struct sockaddr_in cli;
                unsigned len;
                int connfd = accept(sockfd, (struct sockaddr*)&cli, &len);
                if(connfd < 0)
                {
                    _log("accept error");
                    exit(1);
                }
                _log("Client connected, socket:%d, port: %d", connfd, ntohs(cli.sin_port));

                // add to epoll
                event[i].data.fd = connfd;
                event[i].events = EPOLLIN | EPOLLET;
                int ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, event);
                if(ret < 0)
                {
                    _log("epoll_ctl_add error");
                    exit(1);
                }
                continue;
            }
            else
            {
                // client socket ready to read data from
                int clientfd = event[i].data.fd;
                if (event[i].events & EPOLLERR || event[i].events & EPOLLHUP){
                    _log("client connection error(%d)", clientfd);
                }
                else if (event[i].events & EPOLLIN) {
                    handle_connection(clientfd);
                }

                // remove client socket from epoll	 
                struct epoll_event ev;
                ev.events = EPOLLIN;
                ev.data.fd = clientfd;
                epoll_ctl(epollfd, EPOLL_CTL_DEL, clientfd, &ev);

                // close connection
                _log("Client connection closed(%d)", clientfd);
                close(clientfd);
            }
        }
    }
}

// main function 
int main()
{ 
    // create socket, set mode 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        _log("socket creation failed..."); 
        exit(1); 
    } 
    else
        _log("Socket successfully created.."); 

    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK) == -1) {
        return -1;
    }

    // assign IP, PORT 
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT); 

    // bind a socket 
    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) { 
        _log("socket bind failed..."); 
        exit(1); 
    } 
    else
        _log("Socket successfully binded.."); 

    // start lisening for a socket 
    if ((listen(sockfd, 5)) != 0) { 
        _log("Listen failed..."); 
        exit(1); 
    } 
    else
        _log("Server listening.."); 

    // create epoll descriptor 
    epollfd = epoll_create(1);
    if(epollfd < 0 )
    {
        _log("epoll_create error");
        exit(1);
    }
    
    // add server socket to epoll 
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = sockfd;

    int ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev);
    if(ret < 0)
    {
        _log("epoll_ctl error");
        exit(1);
    }

    // create and run worker threads pool 
    pthread_t thread_pool[THREAD_POOL_SIZE];
    for(int i = 0; i < THREAD_POOL_SIZE; i++)
    {
        pthread_create(&thread_pool[i], NULL, thread_func, NULL);
        pthread_join(thread_pool[i], NULL);
    }
}
