#include <iostream>
#include <thread>
#include <vector>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include<bits/stdc++.h>
 
#define LOAD_BALANCER_PORT 8080
using namespace std;
 
pthread_mutex_t mutexhr;
 
#define NUM_EDGE_SERVERS 3
int EDGE_PORT[] = {9090,9091,9092};
 
// vector<pair<int,int>> server_coord(NUM_EDGE_SERVERS);
// vector<int> server_signal_speed(NUM_EDGE_SERVERS);
// vector<int> server_max_connections(NUM_EDGE_SERVERS);
 
vector<int> num_connections(NUM_EDGE_SERVERS,0);
struct edgeserver_info{
    pair<int,int> coordinates;
    int signal_speed;
    int max_no_of_connections;
};
vector<edgeserver_info> server_metadata(NUM_EDGE_SERVERS);
 
vector<vector<int>> coordinates(NUM_EDGE_SERVERS,vector<int>(2));
int getmetadata(){
    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));                 // zero the struct
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  // set destination IP number - localhost, 127.0.0.1
    int serversocket[NUM_EDGE_SERVERS];
    for(int i = 0;i<NUM_EDGE_SERVERS;i++){
        serversocket[i] = socket(AF_INET, SOCK_STREAM, 0);
        dest.sin_port = htons(EDGE_PORT[i]);                    // set destination port number
        int connectResult = connect(serversocket[i], (struct sockaddr *)&dest, sizeof(struct sockaddr_in));
        if (connectResult == -1){
            printf("Client Connection Error: %s\n", strerror(errno));
            return -1;
        }
        int s = 1;
        send(serversocket[i], &s, sizeof(int), 0);
        cout<<"Sent"<<endl;
        recv(serversocket[i], &server_metadata[i], sizeof(edgeserver_info), 0);
        cout<<"Received "<<server_metadata[i].signal_speed<<endl;
        close(serversocket[i]);
    }
}
int choose_server(pair<int,int> client_coords){
    double w1,w2;
    int server = 0;
    double value = 1e10;
    int x = client_coords.first;
    int y = client_coords.second;
    for(int i = 0;i<NUM_EDGE_SERVERS;i++){
        double dist = pow(abs(x-server_metadata[i].coordinates.first),2)+pow(abs(y-server_metadata[i].coordinates.second),2);
        dist = sqrt(dist);
        double prop_delay = dist/server_metadata[i].signal_speed;
        double server_load = ((double)num_connections[i])/server_metadata[i].max_no_of_connections;
        double penalty = w1*prop_delay+w2*server_load;
        if(penalty<value){
            server = i;
            value = penalty;
        }
    }
    return server;
}
void* process_request(void *arguments)
{
    int consocket = *((int *)arguments);    
   
 
    pair<int,int> coords_client = {0,0};
    int server_id;
    recv(consocket, &coords_client, sizeof(pair<int,int>), 0);
    cout<<coords_client.first<<"    "<<coords_client.second<<endl;
    server_id = choose_server(coords_client);
    int port_to_connect = EDGE_PORT[server_id];
    num_connections[server_id]++;
    struct sockaddr_in dest;
    int serversocket = socket(AF_INET, SOCK_STREAM, 0);
    int num_to_send = 2;//sending so that edge server gets that it need not send meta data
    
    memset(&dest, 0, sizeof(dest));                 // zero the struct
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  // set destination IP number - localhost, 127.0.0.1
    dest.sin_port = htons(port_to_connect);                    // set destination port number
    int connectResult = connect(serversocket, (struct sockaddr *)&dest, sizeof(struct sockaddr_in));
    send(serversocket,&num_to_send,sizeof(int),0);
    if (connectResult == -1){
        printf("Client Connection Error: %s\n", strerror(errno));
        return NULL;
    }
    while(true){
        int ack = 1;
        send(consocket, &ack, sizeof(int), 0);
        cout<<"sent ack"<<endl;
        char received_data[1024];
        ssize_t data_size;
        data_size = recv(consocket, &received_data, sizeof(received_data), 0);
        received_data[data_size] = '\0';
        string received_string(received_data);
        cout << "Received data: " << received_string << endl;
        num_to_send = 2;//sending so that edge server gets that it need not send meta data
        send(serversocket,&num_to_send,sizeof(int),0);
        const char *newmsg = received_string.c_str();
        cout<<strlen(newmsg)<<endl;
        send(serversocket, newmsg, strlen(newmsg), 0);
        // pair<int,int> p1;
        // recv(serversocket, &p1, sizeof(pair<int,int>), 0);
        // cout<<p1.first<<" "<<p1.second<<endl;
        // send(consocket, &p1, sizeof(pair<int,int>), 0);
        char dump[1024];
        ssize_t rlen = recv(serversocket,&dump,sizeof(dump),0);
                
        dump[rlen] = '\0';
        string result(dump);
        cout<<result<<endl;
        const char* send_to_client = result.c_str();
        send(consocket,send_to_client,strlen(send_to_client),0);
        int received_data_ack;
        recv(consocket, &received_data_ack, sizeof(int), 0);
        cout<<"Acknowledgement received from client"<<endl;
        

        // send(serversocket, &p1, sizeof(pair<int,int>), 0);
    }
    // send(consocket,&port_to_connect,sizeof(int),0);
    
    close(consocket);
    num_connections[server_id]--;
    return NULL;
}
 
void* waiting_for_connections(void *arguments)
{    
    struct sockaddr_in dest;  
    struct sockaddr_in serv;    
    int clientsocket;          
    socklen_t socksize = sizeof(struct sockaddr_in);
 
    memset(&serv, 0, sizeof(serv));            
    serv.sin_family = AF_INET;            
    serv.sin_addr.s_addr = htonl(INADDR_ANY);  
    serv.sin_port = htons(LOAD_BALANCER_PORT);  
 
    clientsocket = socket(AF_INET, SOCK_STREAM, 0);
 
    bind(clientsocket, (struct sockaddr *)&serv, sizeof(struct sockaddr));
 
    listen(clientsocket, 1);
    printf("Connection Port: %d\n", LOAD_BALANCER_PORT);
   
    int consocket;
    int *aux;
    pthread_t *thread;      
    while (true)
    {
        aux = (int *)calloc(1, sizeof(int));
        thread = (pthread_t *)calloc(1, sizeof(pthread_t));
 
        consocket = accept(clientsocket, (struct sockaddr*)&dest, &socksize);  
        *aux = consocket;    
       
     
        if (pthread_create(thread, NULL, &process_request, aux) != 0)
        {
            printf("Error Creating the Thread.\n");
        }    
    }
 
    return NULL;      
}
 
 
int main(){
 
    pthread_mutex_init(&mutexhr, NULL);
    pthread_t main_thread;
 
    getmetadata();
    if (pthread_create(&main_thread, NULL, &waiting_for_connections, NULL) != 0)
    {
        printf("Error Creating the Thread.\n");
    }
 
    if (pthread_join(main_thread, NULL) != 0)
    {
        printf("Error Joining the Thread.\n");
    }
}