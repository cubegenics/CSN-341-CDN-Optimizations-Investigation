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
#include <bits/stdc++.h>
#include "sem.h"
using namespace std;

#define SERV_PORT 8081
#define SERVER_IP "127.0.0.1"

// #define REQ_CLUSTER_DIFF 100

#include <chrono>
#include <ctime>
#include <iostream>
#include <sys/time.h>

string current_time() {
    // time_t givemetime = time(NULL);
    // string s = ctime(&givemetime);
    // return s;
    struct timeval time_now {};
    gettimeofday(&time_now, nullptr);
    time_t msecs_time = (time_now.tv_sec * 1000) + (time_now.tv_usec / 1000);

    return to_string(msecs_time);
    // cout << "seconds since epoch: " << time_now.tv_sec << endl;
    // cout << "milliseconds since epoch: " << msecs_time << endl << endl;
}

int main(int argc, char **argv){
    
    if (argc != 2) {
        printf("usage: <n/p> for normal/prefetch mode\n");
        exit(0);
    }
    string mode = argv[1];

    // declare variable here
    int listenfd, clilen, connfd, childpid, n1;
    struct sockaddr_in servaddr, cliaddr;
    char msg1[256];
    char temp[256];

    fstream ofs;
    ofs.open("server_load.txt", std::ofstream::out | std::ofstream::trunc);
    ofs.close();
    //the above segment of code clears the content inside the file


    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    //tehelka testing 2...
    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &temp, sizeof(int))==-1){
        perror("setsockopt");
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
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

            string time = current_time();
            auto update_load = [&] () {
                fstream file;
                file.open("server_load.txt", ios::app);
                file << time << '\n';
            };

            update_load();

            close(listenfd);
            n1 = read(connfd, msg1, 256);
            cout<<"request received to send: ";
            printf("%s\n", msg1);

            auto send_prefetch_data = [&] () {
                string s = msg1;
                int v1 = stoi(s);
                int v2 = v1 + rand() % 5;
                int v3 = v1 + rand() % 5;
                
                s = to_string(v1) + ' ' + to_string(v2) + ' ' + to_string(v3);

                bzero(&msg1, 256);
                for (int i = 0; i < (int)s.size(); i++) {
                    msg1[i] = s[i];
                }
                write(connfd, msg1, 50);
            };
            auto send_normal_data = [&] () {
                write(connfd, msg1, 50);
            };


            if (mode == "n") {
                //without prefetch:
                send_normal_data();
            } else {
                //with prefetch:
                send_prefetch_data();
            }

            close(connfd);
            exit(0);
        }
        close(connfd);
    }

}

