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
#include<sys/time.h>
using namespace std;
using ll=long long;

#define server_addr "127.0.0.1"
#define server_port 8080

long long getcurrtime(){
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long long mslong = (long long) tp.tv_sec * 1000L + tp.tv_usec / 1000; //get current timestamp in milliseconds
    // std::cout << mslong << std::endl;
    return mslong;
}


vector<string> send_req(){
    vector<string> input;
    string srg;  
    ifstream filestream("input.txt");  
    if (filestream.is_open())  
    {  
        while ( getline (filestream,srg) )  
        {  
        // cout << srg <<endl;  
            // input+=(" "+srg);
            input.push_back(srg);
        }  
        filestream.close();  
    }  
    else {  
        cout << "File opening is fail."<<endl;   
    } 
    input[0]=input[0].substr(1);
    return input;
}


int main(){

    // ***************************************************************************

    vector<string> input=send_req();
    ll t1=getcurrtime();

    for(int j=0;j<input.size();j++){


        int sockfd, n;
        struct sockaddr_in servaddr;
        char sendline[5000];
        char recvline[5000];
        char temp[5000];
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            printf("socket creation failed...\n");
            cout<<"client: error! exiting...\n";
            exit(0);
        }
        else{
            printf("Socket successfully created..\n");
        }

        //tehelka testing 2...
        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &temp, sizeof(int))==-1){
            perror("setsockopt");
        }

        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(server_port);
        servaddr.sin_addr.s_addr = inet_addr(server_addr);

        if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) !=0){
            printf("connection with the edge server failed...\n");
            exit(0);
        }
        else{
            cout<<"receiver: connected to edge server...\n";
        }
        
        // fgets(sendline, 512, stdin);
        // cout<<input<<"\n";

        bzero(&sendline, sizeof(sendline));
        for(int i=0;i<input[j].length();i++){
            sendline[i]=input[j][i];
        }

        n = strlen(sendline);

        write(sockfd, sendline, n);
        n = read(sockfd, recvline, 512);
        if (n < 0)
            printf("error reading\n");
        recvline[n] = 0;
        cout<<"successfully read data: ";
        fputs(recvline, stdout);
        cout<<"\n";
    }
    // cout<<getcurrtime()<<"\n";
    ll t2=getcurrtime();
    cout<<t2-t1<<"\n";
    exit(0);
}

