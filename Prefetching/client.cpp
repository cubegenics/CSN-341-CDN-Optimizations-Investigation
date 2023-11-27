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
#include <random>
#include <sys/time.h>

static std::random_device rd;
static std::mt19937 gen(rd());
using namespace std;


#define server_port 8080
#define NUM_OF_REQ 500
#define MAX_DATA_VALUE 10000


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

int generate_poisson_random(int lambda) {
    std::poisson_distribution<int> pd(lambda);
    return pd(gen);
    // for (int i = 0; i < 10000; ++i)
    //     std::cout << pd(gen) << '\n';
}

vector<int> generate_trace(int lambda, int threshold) {
    srand(time(0));
    int k_factorial = 1;
    for (int i = 1; i <= threshold; i++) {
        k_factorial *= i;
    }
    vector<int> values;
    int x = rand() % 10 + 1;
    for (int i = 0; i < NUM_OF_REQ; i++) {
        if ((int)values.size() > NUM_OF_REQ) break;
        int v1 = rand() % MAX_DATA_VALUE + 1;
        values.push_back(v1);
        int do_process = rand() % 10 + 1;
        if (do_process <= 8) {
            int random_value = generate_poisson_random(lambda);
            if (random_value <= threshold) {
                int v2 = v1 + rand() % 5;
                int v3 = v1 + rand() % 5;
                values.push_back(v2);
                values.push_back(v3);
            }
        }
    }
    while ((int)values.size() > NUM_OF_REQ) values.pop_back();
    return values;
}

vector<int> generate_trace_updated(int lambda) {
    srand(time(0));
    vector<int> values(NUM_OF_REQ, -1);
    int x = rand() % 10 + 1;
    for (int i = 0; i < NUM_OF_REQ; i++) {
        if (values[i] != -1) continue;
        int v1 = rand() % MAX_DATA_VALUE + 1;
        values[i] = v1;
        int random = rand() % 10 + 1;
        if (random <= 8) {
            for (int j = 0; j < 2; j++) {
                int poisson = generate_poisson_random(lambda);
                int idx = i + poisson;
                if (idx < NUM_OF_REQ && values[idx] == -1) {
                    int v2 = v1 + rand() % 5;
                    values[idx] = v2;
                }
            }
        }
    }
    while ((int)values.size() > NUM_OF_REQ) values.pop_back();
    return values;
}



int main(int argc, char ** argv){

    if (argc != 2) {
        printf("Usage: <ip> of the edge server to connect to\n");
        exit(0);
    } 
    int sockfd, n;
    struct sockaddr_in servaddr;
    char sendline[256];
    char recvline[256];
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        cout<<"client: error! exiting...\n";
        exit(0);
    }
    else{
        printf("Socket successfully created..\n");
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(server_port);
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);

    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) !=0){
        printf("connection with the edge server failed...\n");
        exit(0);
    }
    else{
        cout<<"receiver: connected to edge server...\n";
    }


    int hits = 0;

    vector<int> values = generate_trace_updated(3);

    

    long long client_latency = 0;
    for (auto &value: values) {

        bzero(&sendline, 256);
        auto integer_to_buffer = [&] (int x) {
            string s = to_string(x);
            int n = 0;
            for (int i = 0; i < (int)s.size(); i++) {
                sendline[n] = s[i];
                n++;
            }
            sendline[n] = '\0';
        };

        integer_to_buffer(value);


        n = strlen(sendline);

        write(sockfd, sendline, n);
        string request_timestamp = current_time();

        bzero(&recvline, 256);

        n = read(sockfd, recvline, 256);
        string receive_timestamp = current_time();

        if (n < 0)
            printf("error reading\n");

        recvline[n] = 0;
        cout<<"successfully read data id: ";
        fputs(recvline, stdout);

        if (recvline[0] == 'h') hits++;
        cout << '\n';


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

        client_latency += difference(request_timestamp, receive_timestamp);

    }

    cout << "Hit ratio: " << setprecision(5) << (double)hits / (double)NUM_OF_REQ << '\n';
    cout << "Net Client Latency: " << client_latency << '\n';

    close(sockfd);
    exit(0);
}

