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
#include <sys/wait.h>
#include <filesystem>
#include <fstream>

#define REC_PORT_NO 7070
#define SERVICE_PORT_NO 8080

#define ll long long

using namespace std;


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

//initialize these maps
map<string,int> IP_Priority;
map<string,int> WEBSITE_Priority;
map<string, vector<string>> FILE_Priority; //-->  GOOGLE.COM -> {a.txt, b.txt, a1.jpg, b1.jpg}



vector<string> website_cache={"f1"};
//ACCEPTS UDP REQUESTS FROM CLIENTS
//CREATES A PRIORITY QUEUE FOR THE REQUESTS
//SERVICES THE REQUESTS IN THE ORDER OF PRIORITY USING TCP


// FILENAME/WEBSITE NAME --> [FILE1, FILE2, FILE3, ....]
// IP/IP PREFIX --> PRIORITY  0.8
// WEBSITE --> PRIORITY   0.2

// DATASTRUCTURES:

// GOOGLE.com 4 a.txt b.txt ..
// a.txt kjdhfhroie

#define FILE_DIRECTORY_PATH "/home/pranav/Desktop/FileSystem"
#define CACHE_SIZE 6
#define MAIN_SERVER_IP "192.168.130.3"
#define MAIN_SERVER_PORT 6000
#define SERVICE_QUEUE_SIZE 2

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

struct CustomComparator {
    bool operator()(pair<string,string> a, pair<string,string> b) {
        int suma = IP_Priority[a.first] + WEBSITE_Priority[a.second];
        int sumb = IP_Priority[b.first] + WEBSITE_Priority[b.second];
        return suma < sumb;
    }
};

string retrieveDataFromFile(string filename, string website_name){
    string filepath = FILE_DIRECTORY_PATH;
    filepath = filepath + "/" + website_name + "/" + filename; 
    fstream file(filepath, ios::in);
    string data;
    getline(file,data);
    return data;
}

bool edgeServerContains(string website_name){
    int idx = -1;
    for(int i=0;i<website_cache.size();i++){
        if(website_cache[i]==website_name){
            idx = i;
            break;
        }
    }
    if(idx==-1){
        return 0;
    }
    website_cache.erase(website_cache.begin()+idx);
    website_cache.insert(website_cache.begin(),website_name);
    return 1;
}

void createFile(string filename, string filedata, string website_name){
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

int readFreePortNumber(){
    //READ FILE
    // return 4000;

    fstream file("FreePort.txt");
    ostringstream num;
    num << file.rdbuf();
    file.close();
    string n = num.str();
    if(n==""){
        n = "4000";
    }
    int res = stoi(n);
    int writenum = res+1;
    string writes = to_string(writenum);
    fstream file1("FreePort.txt", ios::out);
    file1<<writes;
    file1.close();
    return res;
}

void reqFile(string website_name){
    int connfd;
    int mainSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in mainServerAddr;
    mainServerAddr.sin_family = AF_INET;
    mainServerAddr.sin_port = htons(MAIN_SERVER_PORT);  
    mainServerAddr.sin_addr.s_addr = inet_addr(MAIN_SERVER_IP);

    connfd = connect(mainSocket, (struct sockaddr*)&mainServerAddr, sizeof(mainServerAddr));    
    if(connfd==-1){
        cout<<"CONNECTION FAILED!\n";
    }
    char sendmsg[1000];
    bzero(sendmsg, sizeof(sendmsg));
    int port_recv_no = readFreePortNumber();
    string sen_mes = "REQ "+website_name+" "+to_string(port_recv_no);
    for(int i=0;i<sen_mes.size();i++){
        sendmsg[i] = sen_mes[i];
    }
    // REQ website_name PORT
    write(mainSocket,sendmsg, sizeof(sendmsg));
    close(connfd);
    int listenfd, clilen;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in servaddr, cliaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port_recv_no);

    if(bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr))<0){
        cout<<"BIND FAILED!\n";
    }
    listen(listenfd,10);
    clilen = sizeof(cliaddr);
    bzero(sendmsg, sizeof(sendmsg));
    connfd = accept(listenfd, (struct sockaddr *)&cliaddr, (socklen_t*)&clilen);
    read(connfd,sendmsg,sizeof(sendmsg));
    string control_message(sendmsg);
    // control msg from main server = <webName numfiles priority>
    vector<string> control = split(control_message);
    ll num_files = stoi(control[1]);
    WEBSITE_Priority[website_name] = stoi(control[2]);

    vector<string> priority_entry;
    for(int iter = 0; iter<num_files; iter++){
        bzero(sendmsg, sizeof(sendmsg));
        read(connfd,sendmsg,sizeof(sendmsg));
        string file_message(sendmsg);
        vector<string> file_vec = split(file_message);
        createFile(file_vec[0], file_vec[1], website_name);
        priority_entry.push_back(file_vec[0]);
    }
    
    FILE_Priority[website_name] = priority_entry;
}


void getWebsiteFromMainServer(string website_name){
    // remove one website from cache if cache is full
    if(website_cache.size()==CACHE_SIZE){
        website_cache.pop_back();
        string directory_path = FILE_DIRECTORY_PATH;
        directory_path += "/"+website_name;
        std::filesystem::remove_all(directory_path);
        std::filesystem::remove(directory_path);
    }
    // request from mainserver
    reqFile(website_name);
    cout<<"RECEIVED FILE FROM MAIN SERVER!\n";
    // add this website required into cache
    website_cache.insert(website_cache.begin(), website_name);
}


int main() {
    FILE_Priority["f1"]={"a.txt"};
    IP_Priority["192.168.42.33"] = 2;
    IP_Priority["192.168.42.3"] = 1;
    string webName = "WEB";
    for(int i=1; i<=20; i++){
        string wbn2 = webName + to_string(i);
        WEBSITE_Priority[wbn2] = i;

        for(int j=1; j<=1; j++){
            FILE_Priority[wbn2].push_back((wbn2+to_string(j)+".txt"));
        }
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    priority_queue<pair<string,string>, vector<pair<string,string>>, CustomComparator> service_order;
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }
    cout<<"UDP Socket created\n";
    struct sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(REC_PORT_NO);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Binding failed");
        return 1;
    }

    char buffer[1024];
    bzero(buffer, sizeof(buffer));
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    while(true){
        ssize_t bytesRead = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, &addrLen);
        if (bytesRead < 0) {
            perror("Receive failed");
            return 1;
        }
        cout<<"Received UDP Packet\n";

        ll ip_numeric=clientAddr.sin_addr.s_addr;
        string clientIP=ipconvert(ip_numeric);
        string message(buffer);


        //MESSAGE TYPE : REQ WEBSITE
        vector<string> mes = split(message);
        if(mes.size()!=2 or mes[0]!="REQ"){
            cout<<"INVALID REQUEST FROM CLIENT!\n";
            continue;
        }
        service_order.push({clientIP,mes[1]});
        cout << "Received request: " << message <<"\n";
        cout << "Client IP address: " << clientIP <<"\n";

        if(service_order.size() == SERVICE_QUEUE_SIZE){
            while(wait(NULL)>0);
            if(fork()==0){
                close(sockfd);
                while(!service_order.empty()){
                    int TCPsocket = socket(AF_INET, SOCK_STREAM, 0);
                    pair<string, string> req = service_order.top();
                    service_order.pop();
                    string IP_client = req.first;
                    string website_name = req.second;
                    cout<<"STARTING TRANSMISSION of "<<website_name<<" for IP "<<IP_client<<"\n";
                    string asd = "";
                    if(setsockopt(TCPsocket,SOL_SOCKET, SO_REUSEADDR, &asd, sizeof(int))==-1){
                        perror("setsockopt");
                    }
                    struct sockaddr_in client_addr;
                    bzero(&client_addr, sizeof(client_addr));
                    client_addr.sin_family = AF_INET;
                    client_addr.sin_port = htons(SERVICE_PORT_NO);
                    client_addr.sin_addr.s_addr = inet_addr(IP_client.c_str());;
                    sleep(1);
                    if(connect(TCPsocket, (struct sockaddr*)&client_addr, sizeof(client_addr))!=0){
                        cout<<"failed conn\n";
                    }
                    
                    if(!edgeServerContains(website_name)){
                        cout<<"File requested not present at edge server! Requesting Main Server!\n";
                        getWebsiteFromMainServer(website_name);
                    }
                    string message = website_name+" "+to_string(FILE_Priority[website_name].size());
                    char task_mes[1000];
                    bzero(task_mes, sizeof(task_mes));
                    for(int i=0;i<message.size();i++){
                        task_mes[i] = message[i];
                    }
                    write(TCPsocket,task_mes,sizeof(task_mes));

                    for(auto file : FILE_Priority[website_name]){
                        message = file+" "+retrieveDataFromFile(file,website_name);
                        char task_mes[1000];
                        bzero(task_mes, sizeof(task_mes));
                        for(int i=0;i<message.size();i++){
                            task_mes[i] = message[i];
                        }
                        write(TCPsocket,task_mes,sizeof(task_mes));
                    }
                    cout<<"Completed service to IP "<<IP_client<<"\n";
                }
                exit(0);
            }
            while(!service_order.empty()){
                service_order.pop();
            }
        }
    }
    close(sockfd);
    return 0;
}
