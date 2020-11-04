#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <set>
#include <sstream>
#include <unordered_set>

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

}

//split received string to vector<string>
vector<string> split(string str, string pattern){
    vector<string> result;

    while (str.size()){
        int index = str.find(pattern);
        if (index != string::npos){
            result.push_back(str.substr(0, index));
            str = str.substr(index + pattern.size());
            if (str.size() == 0){
                result.push_back(str);
            }
        }else{
            result.push_back(str);
            str = "";
        }
    }
    return result;
}


int main(int argc, const char *argv[]){
    /*
    example client input:
    ./client
    Enter country name: Canada
    Enter user ID: 78
    */
   //reference of the socket part is Beej guidance
    create_TCP_client();
    cout << "The client is up and running" << endl;

    if ((connect(serverTcpfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr))) ==-1){
        perror("Error in create tcp socket!");
        exit(1);
    }

    //sendto mainserver
    char usermsg[BUFFLEN] ;
    string name;  
    string countryName;
    string ID;
    cout << "Enter country name:" ;
    cin >> countryName;
    cout << "Enter user ID:";
    cin >> ID;

    //sendto mainserver
    string str = countryName + "," + ID;
    strcpy(usermsg, str.c_str());
    int sendLen = send(serverTcpfd, usermsg, sizeof(usermsg), 0);
    if (sendLen == -1){
        perror("Error in sending message to main server");
        exit(1);
    }
    
    
    cout << "Client has sent User <"<< ID <<"> and <" <<  countryName << "> to Main Server using TCP" << endl;


    //receive from mainserver
    char recvmsg[BUFFLEN];
    int recvLen = recv(serverTcpfd, recvmsg, sizeof(recvmsg), 0);
    if (recvLen == -1){
        perror("Error in receiving from main server");
        close(serverTcpfd);
        exit(1);
    }
    
    //There are four cases of recvmsg
    //1. country not found
    string str2 = recvmsg;
    if (str2 == "None this country"){
        cout << countryName << " has not found" << endl;
    }
    //2. user has not found
    else if (str2 == "USER NOT FOUND"){
        cout << "user <" << ID << "> has not found" << endl;
    }
    else{
    string resString = recvmsg;
    vector<string> indexAndRecommend = split(resString, ",");
    string recommend = indexAndRecommend[0];
    string index = indexAndRecommend[1];
    
    //3. no recommendation
    if (recommend == "No connection"){
        cout << "Client" << index << " has received results from Main Server: there is no recommendation!" << endl;
    }

    //4. connect to everyone
    if (recommend == "has connect every one"){
        cout << "Client" << index << " has received results from Main Server: this user has connect to everyone in this country!" << endl;
    }else{
        cout << "Client" << index << " has received results from Main Server: User <" << recommend << "> is possible friend of User<" << ID << "> in <" << countryName << ">" << endl;
    }
    }
    

    close(serverTcpfd);
  

}