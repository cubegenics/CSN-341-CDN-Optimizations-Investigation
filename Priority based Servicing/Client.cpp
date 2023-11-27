#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <bits/stdc++.h>
#include <unistd.h>
#include <map>
#include <vector>
#include <signal.h>
#include <sys/wait.h>
#include <fstream>
#include <filesystem>

#define ll long long

#define RECV_PORT_NO 8080 //TCP
#define REQ_PORT_NO 7070  //UDP

#define EDGE_SERVER_IP "192.168.42.250"

//REQUESTS CONTENT FROM EDGE SERVER USING UDP
//LISTENS ON A TCP PORT FOR DATA
//ACCEPTS DATA

using namespace std;

#define FILE_DIRECTORY_PATH "/home/pranav/Desktop/ClientData"

bool tcp_completed = 1;
string website_name;
using namespace std;


vector<string> split(string s){
    // s -> "FILENAME1 FILENAME2 ...."
    vector<string> res;
    string temp = "";
    for(int i=0;i<s.size();i++){
        if(s[i]==' '){
            res.push_back(temp);
            temp = "";
        }
        else{
            temp.push_back(s[i]);
        }
    }
    if(temp!=""){
        res.push_back(temp);
    }
    return res;
}

void createFile(string filename, string filedata){
    //CREATE
    string folderPath = FILE_DIRECTORY_PATH;
    folderPath += "/" + website_name;
    if (!std::filesystem::exists(folderPath)) {
        if (std::filesystem::create_directory(folderPath)) {
            std::cout << "Folder created successfully at " << folderPath << std::endl;
        }
    } 
    string filepath = FILE_DIRECTORY_PATH;
    filepath = filepath + "/" + website_name + "/" + filename;
    ofstream regfile(filepath);
    if(regfile.is_open()){
        regfile<<filedata;
        regfile.close();    
    }
    else{
        cout<<"failed!\n";
    }
}

void handle_alarm(int signum){
    exit(0);
}

string ipconvert(ll ip){
    string s = "";
    for(ll i = 0;i<32;i++){
        if(ip%2==1){
            s.push_back('1');
        }
        else{
            s.push_back('0');
        }
        ip/=2;
    }
    reverse(s.begin(),s.end());
    string res = "";
    for(int i=3;i>=0;i--){
        ll ans = 0;
        for(int j=0;j<8;j++){
            ans *= 2;
            if(s[i*8+j]=='1'){
                ans+=1;
            }
        }
        res = res + to_string(ans);
        if(i>0){
            res.push_back('.');
        }
    }
    return res;
}

int readTCPCompleted(){
    fstream file;
    file.open("TCPCompleted.txt", ios::in);
    string s;
    getline(file,s);
    if(s==""){
        return 1;
    }
    return stoi(s);
}

void setTCPCompleted(int val){
    string s = to_string(val);
    fstream file;
    file.open("TCPCompleted.txt", ios::out);
    file<<s;
}


void reqFile(string website_name){
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(REQ_PORT_NO);
    serverAddr.sin_addr.s_addr = inet_addr(EDGE_SERVER_IP);
    string msg = "REQ "+website_name;
    char message[1000];
    bzero(message, sizeof(message));
    for(int i=0;i<msg.size();i++){
        message[i] = msg[i];
    }
    ssize_t bytesSent = sendto(sockfd, message, strlen(message), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if(bytesSent < 0){
        cout<<"ERROR IN SENDING!\n";
    }
    cout<<"UDP REQUEST SENT!\n";
    close(sockfd);
    signal(SIGALRM, handle_alarm);
    alarm(15);
    int listenfd;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in servaddr,cliaddr;
    int clilen, connfd;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(RECV_PORT_NO);
    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    cout<<"OPENED TCP SOCKET!\n";
    listen(listenfd,5);
    clilen = sizeof(cliaddr);
    char msg1[1000];
    bzero(msg1, sizeof(msg1));
    connfd = accept(listenfd, (struct sockaddr *)&cliaddr, (socklen_t*)&clilen);
    if(connfd<0){
        cout<<"GALAT ACCEPT!\n";
    }
    cout<<"ACCEPTED CONNECTION FROM EDGE SERVER!\n";
    ll ip_numeric = cliaddr.sin_addr.s_addr;
    string ip_ = ipconvert(ip_numeric);
    cout<<ip_<<" is the IP of edge server!\n";
    read(connfd,msg1,sizeof(msg1));
    string control_message(msg1);
    vector<string> control = split(control_message);
    ll num_files = stoi(control[1]);
    for(int iter = 0; iter<num_files; iter++){
        bzero(msg1, sizeof(msg1));
        read(connfd,msg1,sizeof(msg1));
        string file_message(msg1);
        vector<string> file_vec = split(file_message);
        createFile(file_vec[0], file_vec[1]);
    }
    tcp_completed = 1;
    setTCPCompleted(1);
    cout<<"Received File Successfully!\n";
    sleep(7);
}


int main(){
    std::filesystem::remove("TCPCompleted.txt");
    while(true){
        if(readTCPCompleted() == 1){
            cout<<"ENTER WEBSITENAME: ";
            cin>>website_name;  
        }
        setTCPCompleted(0);
        if(fork()==0){
            reqFile(website_name);
        }
        while(wait(NULL)>0);
        tcp_completed = readTCPCompleted();
    }
}