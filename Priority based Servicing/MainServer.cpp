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

//service edge servers concurrently
//one TCP socket for accepting requests 


#define MAIN_SERVER_REQ_PORT 6000
#define ll long long

#define FILE_DIRECTORY_PATH "/home/pranav/Desktop/MainFileSystem"
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

string retrieveDataFromFile(string filename, string website_name){
    string filepath = FILE_DIRECTORY_PATH;
    filepath = filepath + "/" + website_name + "/" + filename; 
    fstream file(filepath, ios::in);
    string data;
    getline(file,data);
    return data;
}

map<string,int> WEBSITE_PRIORITY;
map<string,vector<string>> FILE_Priority;



int main(){

    FILE_Priority["f2"] = {"b.txt"};

    struct sockaddr_in servaddr, cliaddr;
    int clilen, connfd, listenfd;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    cout<<"SOCKET CREATED!\n";
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(MAIN_SERVER_REQ_PORT);
    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    cout<<"BINDED!\n";
    listen(listenfd,5);
    char msg[1000];
    for(;;){
        bzero(msg,sizeof(msg));
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, (socklen_t*)&clilen);
        ll ip_numeric = cliaddr.sin_addr.s_addr;
        string edgeServerIP = ipconvert(ip_numeric);
        cout<<"IP OF CONNECTED EDGE SERVER: "<<edgeServerIP<<"\n";
        // char edge_server_IP[INET_ADDRSTRLEN];
        // inet_ntop(AF_INET, &(cliaddr.sin_addr), edge_server_IP, INET_ADDRSTRLEN);
        // string edgeServerIP(edge_server_IP);
        //CREATE NEW PROCESS FOR EACH REQUEST
        read(connfd, msg, sizeof(msg));
        if(fork()==0){
            close(listenfd);
            string mesg(msg);
            vector<string> ctrl_msg = split(mesg);
            int conn_port_no = stoi(ctrl_msg[2]);
            close(connfd);

            int listenfd1 = socket(AF_INET, SOCK_STREAM, 0);
            struct sockaddr_in edgeServerAddr;
            edgeServerAddr.sin_family = AF_INET;
            edgeServerAddr.sin_port = htons(conn_port_no);  
            edgeServerAddr.sin_addr.s_addr = inet_addr(edgeServerIP.c_str());
            sleep(1);
            connfd = connect(listenfd1, (struct sockaddr*)&edgeServerAddr, sizeof(edgeServerAddr));
            string website_name = ctrl_msg[1];
            string message = website_name+" "+to_string(FILE_Priority[website_name].size())+" "+to_string(WEBSITE_PRIORITY[website_name]);
            char task_mes[1000];
            bzero(task_mes, sizeof(task_mes));
            for(int i=0;i<message.size();i++){
                task_mes[i] = message[i];
            }
            write(listenfd1,task_mes,sizeof(task_mes));

            for(auto file : FILE_Priority[website_name]){
                message = file+" "+retrieveDataFromFile(file,website_name);
                char task_mes[1000];
                bzero(task_mes, sizeof(task_mes));
                for(int i=0;i<message.size();i++){
                    task_mes[i] = message[i];
                }
                write(listenfd1,task_mes,sizeof(task_mes));
            }
            cout<<"REQUEST SERVICED!\n";
            close(listenfd1);
            close(connfd);
            exit(0);            
        }
        close(connfd);
    }

}
