#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>
#include<bits/stdc++.h>

using namespace std;

#define CONNECTING_LB_PORT 8080
struct client_info{
    pair<int,int> client_coordinate;
    int port_to_connect;
};

int main(int argc, char *argv[]){
    if (argc != 3)
    { 
        printf("Usage : ./[output_file_name] [x_coord] [y_coord]");
        return -1; 
    }
    client_info client_info1;
    client_info1.client_coordinate = make_pair(atoi(argv[1]),atoi(argv[2]));
    cout<<client_info1.client_coordinate.first<<endl;
    cout<<client_info1.client_coordinate.second<<endl;
    int serversocket;
   struct sockaddr_in dest;

   serversocket = socket(AF_INET, SOCK_STREAM, 0);

   memset(&dest, 0, sizeof(dest));                 // zero the struct
   dest.sin_family = AF_INET;
   dest.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  // set destination IP number - localhost, 127.0.0.1
   dest.sin_port = htons(CONNECTING_LB_PORT);                    // set destination port number

   int connectResult = connect(serversocket, (struct sockaddr *)&dest, sizeof(struct sockaddr_in));

   if (connectResult == -1)
   {
         printf("Client Connection Error: %s\n", strerror(errno));
         return -1;
   }
    int number;
    cout<<"Enter number\n";
    cin>>number;
    // if(number==-1)
    //     break;
    
    send(serversocket, &client_info1.client_coordinate, sizeof(pair<int,int>), 0);
    cout<<"Data sent."<<endl;
    // recv(serversocket, &client_info1.port_to_connect, sizeof(int), 0);
    // cout<<client_info1.port_to_connect<<endl;
    // close(serversocket);

    // dest.sin_port = client_info1.port_to_connect;
    // connectResult = connect(serversocket, (struct sockaddr *)&dest, sizeof(struct sockaddr_in));
    int ack;
    while(true){
        recv(serversocket, &ack, sizeof(int), 0);
        cout<<ack<<endl;
        if(ack==1){
            const char *message; // The string to send
            string msg;
            cin>>msg;
            message = msg.c_str();
            send(serversocket, message, strlen(message), 0);
            // pair<int,int> p1;
            // recv(serversocket, &p1, sizeof(pair<int,int>), 0);
            // cout<<p1.first<<" "<<p1.second<<endl;
            char dump[1024];
            ssize_t rlen = recv(serversocket,&dump,sizeof(dump),0);
                    
            dump[rlen] = '\0';
            string result(dump);
            cout<<result<<endl;
            int received_data = 1;
            send(serversocket, &received_data, sizeof(int), 0);
        }
   }
        close(serversocket);
    return 0;
}
