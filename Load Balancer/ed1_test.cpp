#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <list>
#include <map>
#include <mutex>
#include <thread>
#include <unistd.h>
#include<bits/stdc++.h>
#include<pthread.h>

using namespace std;

pthread_mutex_t mutexhr;

const int EDGE_SERVER_PORT = 9090;
const int MAIN_SERVER_PORT = 8081;
const int LOAD_BALANCER_PORT = 8080;
const int CACHE_SIZE = 100;

// unordered_map<pair<string,string>> cache;

struct edgeserver_info{
    pair<int,int> coordinates;
    int signal_speed;
    int max_no_of_connections;
};

edgeserver_info edge_server_info;

void *process_request(void *arguments){
    int consocket = *((int *)arguments); 






    int num;
    
    recv(consocket, &num, sizeof(int), 0);
    int port_to_connect = 9091;





    if(num==1){
        send(consocket,&edge_server_info,sizeof(edgeserver_info),0);
        cout<<"Sent metadata"<<endl;
        close(consocket);
    }
    else{
        char received_data[1024];
        ssize_t data_size;
        struct sockaddr_in dest;

        int main_server_socket = socket(AF_INET, SOCK_STREAM, 0);

        memset(&dest, 0, sizeof(dest));                 // zero the struct
        dest.sin_family = AF_INET;
        dest.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  // set destination IP number - localhost, 127.0.0.1
        dest.sin_port = htons(MAIN_SERVER_PORT);                    // set destination port number

        int connectResult = connect(main_server_socket, (struct sockaddr *)&dest, sizeof(struct sockaddr_in));

        if (connectResult == -1)
        {
              printf("Client Connection Error: %s\n", strerror(errno));
              return NULL;
        }
        while(true){
            recv(consocket, &num, sizeof(int), 0);
            if(num==1){
                break;
            }

            data_size = recv(consocket, &received_data, sizeof(received_data), 0);
            char* temp = received_data;
            
            cout<<data_size<<endl;
            if(data_size>0){
                received_data[data_size] = '\0';
                string received_string(received_data);
                cout << "Received data: " << received_string << endl;
                // int ack = 1;
                pair<int,int> p1 = {007,777};
                // send(consocket, &p1, sizeof(pair<int,int>), 0);

                char dump[1024];
                // const char *s = received_string.c_str();
                const char *newmsg = received_string.c_str();
                send(main_server_socket,newmsg,strlen(newmsg),0);
                ssize_t rlen = recv(main_server_socket,&dump,sizeof(dump),0);
                // string before(dump);
                dump[rlen] = '\0';
                string result(dump);

                cout<<result<<endl;
                const char* send_to_client = result.c_str();
                send(consocket,send_to_client,strlen(send_to_client),0);
            }
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
    int load_balancer_socket;          
    socklen_t socksize = sizeof(struct sockaddr_in);

    memset(&serv, 0, sizeof(serv));             
    serv.sin_family = AF_INET;             
    serv.sin_addr.s_addr = htonl(INADDR_ANY);   
    serv.sin_port = htons(EDGE_SERVER_PORT);  

    load_balancer_socket = socket(AF_INET, SOCK_STREAM, 0);

    bind(load_balancer_socket, (struct sockaddr *)&serv, sizeof(struct sockaddr));

    listen(load_balancer_socket, 1);
    printf("Connection Port: %d\n", EDGE_SERVER_PORT);
    int consocket;
    int *aux;
    pthread_t *thread;       
    while (true)
    { 
        aux = (int *)calloc(1, sizeof(int));
        thread = (pthread_t *)calloc(1, sizeof(pthread_t));

        consocket = accept(load_balancer_socket, (struct sockaddr*)&dest, &socksize);  
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
int main(){

    pthread_mutex_init(&mutexhr, NULL);
    pthread_t main_thread;

    edge_server_info.coordinates = {7,18};
    edge_server_info.max_no_of_connections = 10;
    edge_server_info.signal_speed = 10;

    if (pthread_create(&main_thread, NULL, &waiting_for_connections, NULL) != 0)
    {
        printf("Error Creating the Thread.\n");
    }

    if (pthread_join(main_thread, NULL) != 0)
    {
        printf("Error Joining the Thread.\n");
    }
}