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
#include <sys/time.h>



using namespace std;


#define NUM_OF_REQ 200
#define MAX_CACHE_SIZE 20
#define EVICT_COUNT 10

#define SERVER_IP "192.168.42.250"
// #define SERVER_IP "127.0.0.1"
#define SERV_PORT 8080
#define server_port_1 8081

#include <chrono>
#include <ctime>
#include <iostream>

string current_time() {
    struct timeval time_now {};
    gettimeofday(&time_now, nullptr);
    time_t msecs_time = (time_now.tv_sec * 1000) + (time_now.tv_usec / 1000);

    return to_string(msecs_time);
}

int main(int argc, char ** argv){

    if (argc != 4) {
        printf("Usage: <ip> <cache file name> <latency file name>\n");
        exit(0);
    }

    string cache_file_name = argv[2]; //cache file must be initially populated with "0 0"
    string latency_file_name = argv[3]; //latency file must be initially populated with "0"

    
    fstream ofs1;
    ofs1.open(cache_file_name, std::ofstream::out | std::ofstream::trunc);
    ofs1.close();
    //the above segment of code clears the content inside the file

    fstream cache;
    cache.open(cache_file_name, ios::out);
    cache << "0 0" << '\n';

    

    int listenfd, clilen, connfd, childpid, n1;
    struct sockaddr_in servaddr, cliaddr;
    char msg1[256];
    char temp[256];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &temp, sizeof(int))==-1){
        perror("setsockopt");
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // servaddr.sin_addr.s_addr = inet_addr(argv[1]);
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
            for (int i = 0; i < NUM_OF_REQ; i++) {
                close(listenfd);
                bzero(&msg1, 256);
                n1 = read(connfd, msg1, 256);


                // ****************************************************************************
                //communicating with main server

                string s = msg1;           
                // cout << "string: " << s << '\n';
                if (s.empty() || s == " " || s == "\n") {
                    //debug carefully
                    break;
                }
                int value = stoi(s);

                auto present_in_file = [&] (int value) {
                    //also updates the timestamp in case it's a hit
                    fstream cache;
                    cache.open(cache_file_name, ios::in);
                    string s;
                    bool ok = 0;
                    while (!cache.eof()) {
                        getline(cache, s);
                        if (s.empty()) break;
                        int i = 0;
                        while (i < (int)s.size() && s[i] != ' ') i++;
                        s = s.substr(0, i);
                        if (s.empty()) break;
                        int x = stoi(s);
                        if (x == value) {
                            ok = 1;
                            break;
                        }
                    }
                    cache.close();
                    if (ok) {
                        cache.open(cache_file_name, ios::in);
                        vector<string> values;
                        while (!cache.eof()) {
                            getline(cache, s);
                            if (s.empty() || s == " " || s == "\n") break;
                            values.push_back(s);
                        }
                        for (auto &s: values) {
                            int i = 0;
                            while (i < (int)s.size() && s[i] != ' ') i++;
                            int x = stoi(s.substr(0, i));
                            if (x == value) {
                                string time = current_time();
                                string new_s = to_string(x) + ' ' + time;
                                s = new_s;
                                break;
                            }
                        }
                        cache.close();

                        fstream ofs;
                        ofs.open(cache_file_name, std::ofstream::out | std::ofstream::trunc);
                        ofs.close();
                        //the above segment of code clears the content inside the file

                        cache.open(cache_file_name, ios::out);
                        for (auto &s: values) {
                            if (s.empty() || s == " ") continue;
                            cache << s << '\n';
                        }
                        return 1;
                    }

                    return 0;
                };

                int found = present_in_file(value);
                if (!found) {

                    auto evict = [&] () {
                        //evict values from the cache using LRU
                        vector<pair<string, int>> vec;
                        fstream cache;
                        cache.open(cache_file_name, ios::in);

                        string s;
                        while (!cache.eof()) {
                            getline(cache, s);
                            if (s.empty()) break;
                            string num = "";
                            int i = 0;
                            while (i < (int)s.size() && s[i] != ' ') {
                                num += s[i];
                                i++;
                            }
                            s = s.substr(i);
                            if (num.empty() || num == " " || num == "\n") break;
                            int value = stoi(num);
                            vec.push_back({s, value});
                        }

                        cache.close();

                        sort(vec.begin(), vec.end());
                        reverse(vec.begin(), vec.end());

                        for (int i = 0; i < EVICT_COUNT; i++) vec.pop_back();

                        fstream ofs;
                        ofs.open(cache_file_name, std::ofstream::out | std::ofstream::trunc);
                        ofs.close();
                        //the above segment of code clears the content inside the file

                        cache.open(cache_file_name, ios::out);

                        for (auto [s, value]: vec) {
                            string total = to_string(value) + s;
                            cache << total << '\n';
                        }
                    };

                    auto check_evict = [&] () {
                        fstream cache;
                        cache.open(cache_file_name, ios::in);
                        int size = 0;
                        string s;
                        while (!cache.eof()) {
                            getline(cache, s);
                            size++;
                        }
                        if (size >= MAX_CACHE_SIZE) {
                            evict();
                        } else {
                            return;
                        }
                    };

                    check_evict();

                    auto update = [&] (int value) {
                        fstream cache;
                        cache.open(cache_file_name, ios::app);
                        string time = current_time();
                        cache << value << ' ' << time << '\n';
                    };

                    update(value);

                    int sockfd_1, n_1;
                    struct sockaddr_in servaddr_1;
                    char sendline_1[256];
                    char recvline_1[256];
                    sockfd_1 = socket(AF_INET, SOCK_STREAM, 0);
                    if (sockfd_1 == -1) {
                        printf("socket creation failed...\n");
                        cout<<"edge server: error! exiting...\n";
                        exit(0);
                    }
                    else{
                        printf("Socket successfully created..\n");
                    }

                    bzero(&servaddr_1, sizeof(servaddr_1));
                    servaddr_1.sin_family = AF_INET;
                    servaddr_1.sin_port = htons(server_port_1);
                    servaddr_1.sin_addr.s_addr = inet_addr(SERVER_IP);

                    if(connect(sockfd_1, (struct sockaddr *)&servaddr_1, sizeof(servaddr_1)) !=0){
                        printf("connection with the main server failed...\n");
                        exit(0);
                    }
                    else{
                        cout<<"receiver: connected to the main server...\n";
                    }
                    
                    //measure RTT
                     
                    auto difference = [&] (string a, string b) {
                        int i = 0; 
                        //assuming a.size() == b.size()
                        while (i < (int)a.size() && a[i] == b[i]) i++;
                        a = a.substr(i);
                        b = b.substr(i);
                        if (a.empty() || b.empty()) {
                            return 0ll;
                        }
                        long long dif = abs(stoll(b) - stoll(a));
                        return dif;
                    };

                    // fgets(sendline_1, 256, stdin);
                    bzero(&sendline_1, 256);
                    strcpy(sendline_1, msg1);
                    n_1 = strlen(sendline_1);
                    
                    string send_timestamp = current_time();
                    write(sockfd_1, sendline_1, n_1);

                    bzero(&recvline_1, 256);

                    n_1 = read(sockfd_1, recvline_1, 256);
                    string receive_timestamp = current_time();


                    //update latency in the file specific to this edge server
                    long long request_latency = difference(send_timestamp, receive_timestamp);
                    fstream file;
                    file.open(latency_file_name, ios::in | ios::out);

                    string current_latency; 
                    getline(file, current_latency);

                    long long updated_latency = request_latency + stoll(current_latency);

                    file.seekg(0, file.beg);
                    file << updated_latency << '\n';
                    file.close();


                    if (n_1 < 0)
                        printf("error reading\n");
                    recvline_1[n_1] = 0;
                    cout<<"successfully received data id: ";
                    fputs(recvline_1, stdout);
                    cout << '\n';
                    // string server_data(recvline_1);
                    bzero(&msg1, sizeof(msg1));

                    string s = recvline_1;
                    int i = 0;
                    while (i < (int)s.size() && s[i] != ' ') i++;
                    if (i == (int)s.size()) {
                        //single number

                    } else {
                        //three numbers
                        int v1 = stoi(s.substr(0, i));
                        s = s.substr(i + 1);
                        i = 0;
                        while (i < (int)s.size() && s[i] != ' ') i++;
                        int v2 = stoi(s.substr(0, i));
                        s = s.substr(i + 1);
                        i = 0;
                        int v3 = stoi(s);
                        //have already updated v1 above
                        update(v2);
                        update(v3);

                        bzero(&recvline_1, 256);
                        s = to_string(v1);
                        for (int i = 0; i < (int)s.size(); i++) {
                            recvline_1[i] = s[i];
                        }
                    }

                    strcpy(msg1, recvline_1);
                    
                    // exit(0);
                } else {
                    char buffer[256] = "hit";
                    bzero(&msg1, 256);
                    strcpy(msg1, buffer);

                    cout << "HIT! Found data requested by client!: " << '\n';
                }

                // ******************************************************************
                // sending back to client

                write(connfd, msg1, 50);

            }
            close(connfd);
            exit(0);
        }
        close(connfd);
    }
}

