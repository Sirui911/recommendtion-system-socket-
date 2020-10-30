#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <errno.h>
#include <string.h>
//#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <vector>
//#include <map>
#include <unordered_map>
#include <string>
#include <set>
//#include <algorithm>
#include <sstream>
//#include <typeinfo>
#include <unordered_set>
//#include <bits/stdc++.h>

using namespace std;
#define SERVERPORT 33518
#define BUFFLEN 1000

int serverTcpfd;
struct sockaddr_in serverAddr, clientAddr;

void create_TCP_client(){
    serverTcpfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverTcpfd == -1){
        perror("Error in create TCP socket");
        exit(1);
    }
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(33518);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    cout << "The client is up and running" << endl;


    
}


int main(int argc, const char *argv[]){
    /*
    example client input:
    ./client
    Enter country name: Canada
    Enter user ID: 78
    */
    create_TCP_client();

    if ((connect(serverTcpfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr))) ==-1){
        perror("Error in create tcp socket!");
        exit(1);
    }

    //sendto mainserver
    char usermsg[BUFFLEN];
    string name;  
    string countryName;
    string ID;
    cout << "Enter country name:" ;
    cin >> countryName;
    cout << "Enter user ID:";
    cin >> ID;
    //std::cout << "Hello, World!\n";
    cout << "name is:" << countryName << "id is " << ID << endl;

    //sendto mainserver
    string str = countryName + "," + ID;
    strcpy(usermsg, str.c_str());
    int sendLen = send(serverTcpfd, usermsg, sizeof(usermsg), 0);
    if (sendLen == -1){
        perror("Error in sending message to main server");
        exit(1);
    }
    //how to identify client 1 and client 2
    cout << "Client has sent User "<< ID <<"and " <<  countryName << "to Main Server using TCP" << endl;


    //receive from mainserver
    char recvmsg[BUFFLEN];
    int recvLen = recv(serverTcpfd, recvmsg, sizeof(recvmsg), 0);
    if (recvLen == -1){
        perror("Error in receiving from main server");
        close(serverTcpfd);
        exit(1);
    }
    //not finish don't know the format of receive message.



  

}