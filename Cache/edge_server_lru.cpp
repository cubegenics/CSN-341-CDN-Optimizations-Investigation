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

#define SERV_PORT 8080
// #define server_addr_1 "192.168.42.250"
#define server_addr_1 "127.0.0.2"
#define server_port_1 8081

int time_elapsed=0;

vector<int> getInput(string s){
    vector<int> ans;
    string temp="";
    for(int i=0;i<s.length();i++){
        if(s[i]!=' '){
            temp+=s[i];
        }
        else{
            ans.push_back(stoi(temp));
            temp="";
        }
    }
    if(temp!=""){
        ans.push_back(stoi(temp));
        temp="";
    }
    return ans;
}


string ask_from_server(string ask){
    int sockfd_1, n_1;
    struct sockaddr_in servaddr_1;
    char sendline_1[512];
    char recvline_1[512];
    sockfd_1 = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_1 == -1) {
        printf("socket creation failed...\n");
        cout<<"edge server: error! exiting...\n";
        exit(0);
    }
    else{
        // printf("Socket successfully created..\n");
    }

    bzero(&servaddr_1, sizeof(servaddr_1));
    servaddr_1.sin_family = AF_INET;
    servaddr_1.sin_port = htons(server_port_1);
    servaddr_1.sin_addr.s_addr = inet_addr(server_addr_1);

    if(connect(sockfd_1, (struct sockaddr *)&servaddr_1, sizeof(servaddr_1)) !=0){
        printf("connection with the main server failed...\n");
        exit(0);
    }
    else{
        // cout<<"receiver: connected to the main server...\n";
    }

    bzero(&sendline_1, sizeof(sendline_1));
    for(int i=0;i<ask.length();i++){
        sendline_1[i]=ask[i];
    }
    // strcpy(sendline_1, msg1);
    n_1 = strlen(sendline_1);
    
    write(sockfd_1, sendline_1, n_1);
    n_1 = read(sockfd_1, recvline_1, 512);
    if (n_1 < 0) printf("error reading\n");
    recvline_1[n_1] = 0;
    string server_rec(recvline_1);
    return server_rec;
}

// ******************************************************************************************
//cache functions

const int CACHE_SIZE = 64;       // Size of the cache
const int SET_ASSOCIATIVITY = 4; // 4-way set associative cache
const int MAX_TTL = 300;         // Maximum TTL value for cache blocks in seconds considering client request is coming each second
const int BLOCK_SIZE = 4;

int count1=0;

struct CacheLine
{
    bool valid;
    unsigned int tag;
    int data;
    int ttl; // Time-To-Live counter
};

// Main memory
// int mainMemory[256];

// Cache data structure
vector<vector<CacheLine>> cache(CACHE_SIZE / SET_ASSOCIATIVITY, vector<CacheLine>(SET_ASSOCIATIVITY));

// Queue to maintain LRU order for each set
vector<queue<int>> lruQueues(CACHE_SIZE / SET_ASSOCIATIVITY, queue<int>());

// Hit and miss counters
int cacheHits = 0;
int cacheMisses = 0;

// Function to initialize the cache and memory arrays
void initializeCacheAndMemory()
{
    for (int i = 0; i < CACHE_SIZE; i++)
    {
        cache[i / SET_ASSOCIATIVITY][i % SET_ASSOCIATIVITY].valid = false;
        cache[i / SET_ASSOCIATIVITY][i % SET_ASSOCIATIVITY].tag = 0;
        cache[i / SET_ASSOCIATIVITY][i % SET_ASSOCIATIVITY].data = 0;
        cache[i / SET_ASSOCIATIVITY][i % SET_ASSOCIATIVITY].ttl = 0;
    }

    // for (int i = 0; i < 256; i++)
    // {
    //     mainMemory[i] = i * 2; // Initialize main memory with some data
    // }
}

// Function to handle cache hit and update LRU order
void handleCacheHit(int setIndex, int accessedIndex)
{
    cacheHits++;
    std::queue<int> &lruQueue = lruQueues[setIndex];
    std::queue<int> tempQueue;
    while (!lruQueue.empty())
    {
        int front = lruQueue.front();
        lruQueue.pop();
        if (front != accessedIndex)
        {
            tempQueue.push(front);
        }
    }
    lruQueue.push(accessedIndex);
    while (!tempQueue.empty())
    {
        lruQueue.push(tempQueue.front());
        tempQueue.pop();
    }
}

// Function to handle cache miss and update LRU order
void handleCacheMiss(int setIndex, int address, int ttlValue)
{
    string to_send=ask_from_server(to_string(address));
    cacheMisses++;
    std::queue<int> &lruQueue = lruQueues[setIndex];
    int lruIndex = lruQueue.front();
    lruQueue.pop();
    cache[setIndex][lruIndex].valid = true;
    cache[setIndex][lruIndex].tag = address / CACHE_SIZE;
    cache[setIndex][lruIndex].data = stoi(to_send);
    cache[setIndex][lruIndex].ttl = ttlValue; // Set TTL value
    lruQueue.push(lruIndex);
}

// Function to simulate cache read operation with LRU replacement policy and TTL
int readFromCache(int address, int ttlValue)
{
    unsigned int tag = address / CACHE_SIZE;
    int setIndex = (address % CACHE_SIZE) / SET_ASSOCIATIVITY;

    // Check if the data is in the cache
    for (int i = 0; i < SET_ASSOCIATIVITY; i++)
    {
        if (cache[setIndex][i].valid && cache[setIndex][i].tag == tag)
        {
            // Cache hit
            handleCacheHit(setIndex, i);
            return cache[setIndex][i].data;
        }
    }

    // Cache miss
    handleCacheMiss(setIndex, address, ttlValue);
    return cache[setIndex][lruQueues[setIndex].back()].data;
}

// Function to update TTL values for cache entries and invalidate when TTL reaches 0
void updateTTL()
{
    for (int i = 0; i < CACHE_SIZE; i++)
    {
        for (int j = 0; j < SET_ASSOCIATIVITY; j++)
        {
            if (cache[i / SET_ASSOCIATIVITY][j].valid)
            {
                if (cache[i / SET_ASSOCIATIVITY][j].ttl > 0)
                {
                    cache[i / SET_ASSOCIATIVITY][j].ttl--;
                    if (cache[i / SET_ASSOCIATIVITY][j].ttl == 0)
                    {
                        cache[i / SET_ASSOCIATIVITY][j].valid = false;
                    }
                }
            }
        }
    }
}

string handle_req(vector<int> &input, int initialTTL){
    for (int address = 0; address < input.size(); address++)
    {
        int data = readFromCache(input[address] / BLOCK_SIZE, initialTTL);
        updateTTL(); // Update TTL values after each access
        count1++;
        return to_string(data);
    }
    return "error";

    // // Calculate hit and miss ratios
    // double hitRatio = static_cast<double>(cacheHits) / (cacheHits + cacheMisses);
    // double missRatio = static_cast<double>(cacheMisses) / (cacheHits + cacheMisses);

    // std::cout << "Cache Hits: " << cacheHits << std::endl;
    // std::cout << "Cache Misses: " << cacheMisses << std::endl;
    // std::cout << "Hit Ratio: " << hitRatio << std::endl;
    // std::cout << "Miss Ratio: " << missRatio << std::endl;
}






int main(){
    // ******************************************************************************************
    // initial cache work

    initializeCacheAndMemory();
    // Initialize the LRU queues with initial order for each set
    for (int i = 0; i < CACHE_SIZE / SET_ASSOCIATIVITY; i++){
        for (int j = 0; j < SET_ASSOCIATIVITY; j++){
            lruQueues[i].push(j);
        }
    }

    // TTL value for new cache entries
    int initialTTL = MAX_TTL;




// **************************************************************************************************
    // declare variable here
    int listenfd, clilen, connfd, childpid, n1;
    struct sockaddr_in servaddr, cliaddr;
    char msg1[5000];
    char temp[5000];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    //tehelka testing 2...
    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &temp, sizeof(int))==-1){
        perror("setsockopt");
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    // servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(SERV_PORT);
    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    printf("binded...\n");
    listen(listenfd, 5);
    printf("listening....\n");
    for (;;)
    {
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, (unsigned int*)&clilen);
        printf("connected...\n");
        bzero(msg1, sizeof(msg1));
        n1 = read(connfd, msg1, 5000);
        string req_list(msg1);
        // cout<<req_list<<" "<<req_list.length()<<"\n";
        vector<int> input=getInput(req_list);
        // for(int i=0;i<input.size();i++){
        //     cout<<input[i]<<" ";
        // }
        // cout<<"\n";

        
        string final_message=handle_req(input, initialTTL);

        
        

        // ******************************************************************
        // sending back to client
        bzero(&msg1, sizeof(msg1));
        for(int i=0;i<final_message.length();i++){
            msg1[i]=final_message[i];
        }
        write(connfd, msg1, 50);
        close(connfd);
        time_elapsed++;
        if(time_elapsed%1000==0){
            // Calculate hit and miss ratios
            double hitRatio = static_cast<double>(cacheHits) / (cacheHits + cacheMisses);
            double missRatio = static_cast<double>(cacheMisses) / (cacheHits + cacheMisses);

            std::cout << "Cache Hits: " << cacheHits << std::endl;
            std::cout << "Cache Misses: " << cacheMisses << std::endl;
            std::cout << "Hit Ratio: " << hitRatio << std::endl;
            std::cout << "Miss Ratio: " << missRatio << std::endl;
        }
    }


}

