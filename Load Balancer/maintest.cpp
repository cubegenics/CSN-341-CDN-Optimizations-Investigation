#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <map>
#include <unistd.h>
#include<bits/stdc++.h>
#include<pthread.h>
#include<mutex>
const int MAIN_SERVER_PORT = 8081;

using namespace std;
pthread_mutex_t mutexhr;

map<string,string> database;

void *process_request(void *arguments){
    int consocket = *((int *)arguments);     
    int num;
    

    
    char received_data[1024];
    ssize_t data_size;
    
    while(true){
        data_size = recv(consocket, &received_data, sizeof(received_data), 0);
        cout<<data_size<<endl;
        if(data_size>0){
            received_data[data_size] = '\0';
            string received_string(received_data);
            cout << "Received data: " << received_string << endl;
            string result="not_present";
            if(database.find(received_string)!=database.end()){
                result=database[received_string];

            }
            
            const char* data_to_forward = result.c_str();
            send(consocket,data_to_forward,strlen(data_to_forward),0);
        }
    }
    
    // else{
    // while(true){
	// recv(consocket, &coords_client, sizeof(pair<int,int>), 0);
	
    // cout<<coords_client.first<<endl;
    // send(consocket,&port_to_connect,sizeof(int),0);
    // }
    // }
    close(consocket);
    return NULL;
}

void* waiting_for_connections(void* arguments){
    struct sockaddr_in dest;   
    struct sockaddr_in serv;     
    int edge_server_socket;          
    socklen_t socksize = sizeof(struct sockaddr_in);

    memset(&serv, 0, sizeof(serv));             
    serv.sin_family = AF_INET;             
    serv.sin_addr.s_addr = htonl(INADDR_ANY);   
    serv.sin_port = htons(MAIN_SERVER_PORT);  

    edge_server_socket = socket(AF_INET, SOCK_STREAM, 0);

    bind(edge_server_socket, (struct sockaddr *)&serv, sizeof(struct sockaddr));

    listen(edge_server_socket, 1);
    printf("Connection Port: %d\n", MAIN_SERVER_PORT);
    int consocket;
    int *aux;
    pthread_t *thread;       
    while (true)
    { 
        aux = (int *)calloc(1, sizeof(int));
        thread = (pthread_t *)calloc(1, sizeof(pthread_t));

        consocket = accept(edge_server_socket, (struct sockaddr*)&dest, &socksize);  
        *aux = consocket;     
        
        cout<<"New thread created"<<endl;
        if (pthread_create(thread, NULL, &process_request, aux) != 0)
        {
            printf("Error Creating the Thread.\n");
        }     
    }

    return NULL;      
    return 0;
}
void setdata(){
    database["Web_series1"]="author1";
    database["Web_series2"]="author2";
    database["Web_series3"]="author3";
    database["Web_series4"]="author4";
    database["Web_series5"]="author5";
    database["Web_series6"]="author6";
    database["Web_series7"]="author7";
    database["Web_series8"]="author8";
    database["Web_series9"]="author9";
    database["Web_series10"]="author10";
    database["Web_series11"]="author11";
    database["Web_series12"]="author12";
}

int main(){

    pthread_mutex_init(&mutexhr, NULL);
    pthread_t main_thread;
    
    setdata();

    if (pthread_create(&main_thread, NULL, &waiting_for_connections, NULL) != 0)
    {
        printf("Error Creating the Thread.\n");
    }

    if (pthread_join(main_thread, NULL) != 0)
    {
        printf("Error Joining the Thread.\n");
    }
    return 1;
}