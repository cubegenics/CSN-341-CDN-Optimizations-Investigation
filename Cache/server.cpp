#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h> 
#include <strings.h> 
#include <sys/wait.h> 
#include <filesystem>
#include <iostream>  
#include <fstream>
#include <signal.h>
#include<bits/stdc++.h>
using namespace std;

#define SERV_PORT 8081

int mainMemory[256];

void init(){
    for(int i=0;i<256;i++){
        mainMemory[i]=2*i;
    }
}

int main(){
    init();
    // declare variable here
    int listenfd, clilen, connfd, childpid, n1;
    struct sockaddr_in servaddr, cliaddr;
    char msg1[512];
    char temp[512];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    //tehelka testing 2...
    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &temp, sizeof(int))==-1){
        perror("setsockopt");
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    // servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.2");
    servaddr.sin_port = htons(SERV_PORT);
    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    printf("binded...\n");
    listen(listenfd, 5);
    printf("listening....\n");
    for (;;)
    {
        bzero(msg1, sizeof(msg1));
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, (unsigned int*)&clilen);
        printf("connected...\n");
        if ((childpid = fork()) == 0)
        {
            close(listenfd);
            n1 = read(connfd, msg1, 512);
            cout<<"Cache miss on edge server. ";
            cout<<"Request received to send block: ";
            printf("%s\n", msg1);
            string received(msg1);
            bzero(&msg1, sizeof(msg1));

            string data_d=to_string(mainMemory[stoi(received)]);
            for(int i=0;i<data_d.length();i++){
                msg1[i]=data_d[i];
            }
            write(connfd, msg1, 50);
            close(connfd);
            exit(0);
        }
        close(connfd);
    }
}

