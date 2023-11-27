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

/*
data values pairs on which to observe:
NUM_OF_REQ, MAX_DATA_VALUE, CACHE_SIZE
100, 1000, 10
500, 1000, 30
500, 10000, 30
*/

#define server_port 8080
#define NUM_OF_REQ 500
#define MAX_DATA_VALUE 10000

void update_results() {
    /*
    a message from an edge server to the client would have the following pieces of information:
    i) data requested
    ii) whether the edge server got that data from its cache or the main server -> can be used to measure the overall latency in content delivery and hit rates for the cache
    we need a factor to estimate how much time we saved from prefetching, rather than fetching data again and again from the main server
    so, we need to analyze the client requests in the form of clusters of related data.
    say, the data has 5 packets
    without prefetching, we'd need to fetch the data 5 times from the main server
    with prefetching, we'd need to fetch the data less than 5 times from the main server, adding
    to time saved from RTT's between the edge server and the main server

    
    iii) to analyze the load on the main server,
    what we could do is save the timestamps of the requests served 
    any set of requests served within a given duration of time would 
    be assumed to be requested from the main server at the same time
    so, we could plot the number of requests fulfilled by the edge
    server in that duration and plot the graph for the load on the
    main server
>
    iv) also want to analyze the link congestion on the links from the edge server to the main server
    to accomplish this, we could create files specific to edge servers that we've serviced according to their IP's
    in these edge server specific files, we'd have information about the timestamps of their requests to the main server
    we'd then cluster these requests into groups of a certain size (with respect to difference in timestamp at which they were served)
    for e.x. group requests served within 200ms of each other
    with this, we'd say that the more the requests served in a group, the worse the link congestion on the link from the main server to that edge server

    v) RTT Delay would be measured per edge server on the basis of how many times it needed to request data from the main server
    it'd have a timestamp for when it requested the data and one for when it received it
    the difference between these times would be the time required to retreive the data from the main server
    then, for the final evaluation, you could just add all the RTT's for each edge server and display the per-edge server delay
    in files specific to each of the edge servers


    aspects whose analysis is complete:
    - Cache Hits (displayed in the client)
    - Server Load (run manually after running the requests from all clients: server_load.cpp)
    - RTT (in latency files: l1.txt, l2.txt, l3.txt)

    remaining: Link Congestion
>
    */
}

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
    // for (int i = 0; i < (NUM_OF_REQ + 2)/ 3; i++) {
    //     int v1 = rand() % MAX_DATA_VALUE + 1;
    //     int v2, v3;
    //     if (v1 % 3 == 0) {
    //         v2 = v1 + 1;
    //         v3 = v1 + 2;
    //     } else if (v1 % 3 == 1) {
    //         v2 = v1 - 1;
    //         v3 = v1 + 1;
    //     } else {
    //         v2 = v1 - 1;
    //         v3 = v1 - 2;
    //     }
    //     values.push_back(v1);
    //     values.push_back(v2);
    //     values.push_back(v3);
    // }
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


    //performance

    int hits = 0;

    //performance end..


    // automate input..

    // vector<int> values = generate_trace(5, 6);
    vector<int> values = generate_trace_updated(3);
    // vector<int> values;
    // for (int i = 0; i < NUM_OF_REQ; i++) {
    //     values.push_back(rand() % MAX_DATA_VALUE + 1);
    // }
    

    //automate input end..

    long long client_latency = 0;
    // for (;;) {
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

        // cout << "Enter message: ";
        // fgets(sendline, 256, stdin);
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

        // fstream file;
        // file.open("client_latency.txt", ios::out);
        // file << difference(request_timestamp, receive_timestamp) << '\n';
        client_latency += difference(request_timestamp, receive_timestamp);

    }

    cout << "Hit ratio: " << setprecision(5) << (double)hits / (double)NUM_OF_REQ << '\n';
    cout << "Net Client Latency: " << client_latency << '\n';

    close(sockfd);
    exit(0);
}

