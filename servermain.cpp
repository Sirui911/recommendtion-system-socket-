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

#define TCPPORT 33518
#define UDPPORT 32518
#define serverA_UDP_PORT 30518
#define serverB_UDP_PORT 31518
#define BUFFLEN 1000
#define BACKLOG 2

struct sockaddr_in mainSerUDP, UDPClientAddr;
struct sockaddr_in mainSerTCP, TCPclientAddr;
int mainSerUDPfd, mainSerTCPfd;

unordered_map<string, int> country_server;
vector<string> listA, listB;
string resA, resB;
unordered_set<string> countrySetA, countrySetB;
string countryName, userId;
pid_t pid;
string request;
string flag = "1";

vector<string> split(string str, string pattern){
    //string::size_type pos;
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

//initiate UDP socket
void create_UDP_socket(){
    //-----------1. create UDP-------------------------
    //int serverAfd = 0;
    
    if ((mainSerUDPfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        perror("Cannot create UDP connection");
        exit(1);
    }
    
    
    //mainSerUDPfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&mainSerUDP, 0, sizeof(mainSerUDP));
    mainSerUDP.sin_family = AF_INET;
    mainSerUDP.sin_port = htons(UDPPORT);
    mainSerUDP.sin_addr.s_addr = inet_addr("127.0.0.1");


    // //-----------2. bind UDP----------------------------
    // bind(mainSerUDPfd, (struct sockaddr *)&mainSerUDP, sizeof(mainSerUDP));
    if (bind(mainSerUDPfd, (struct sockaddr *)&mainSerUDP, sizeof(mainSerUDP)) < 0){
        cout<<"equals to -1"<<endl;
        close(mainSerUDPfd);
        perror("cannot bind the UDP socket");
        exit(1);
    }

}

void create_TCP_socket(){
    //1.create tcp socket
    mainSerTCPfd = socket(AF_INET, SOCK_STREAM, 0);
    if (mainSerTCPfd == -1){
        perror("Error in creating TCP socket with client");
        exit(1);
    }

    int on = 1;
    if (setsockopt(mainSerTCPfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1){
        perror("Error in setsockopt");
        exit(1);
    }

    mainSerTCP.sin_family = AF_INET;
    mainSerTCP.sin_port = htons(TCPPORT);
    mainSerTCP.sin_addr.s_addr = inet_addr("127.0.0.1");

    //2. bind tcp socket
    if (bind(mainSerTCPfd, (struct sockaddr *)&mainSerTCP, sizeof(mainSerTCP)) == -1){
        perror("Error in binding TCP socket");
        close(mainSerTCPfd);
        exit(1);
    }
    //3.listen to
    if (listen(mainSerTCPfd, BACKLOG) == -1){
        perror("Error in listening");
        exit(1);
    }

}


//set port and IP of two servers
// void config_server(){
//     //set server A sockaddr_in
//     serverAaddr.sin_family = AF_INET;
//     serverAaddr.sin_port = htons(serverA_UDP_PORT);
//     serverAaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

//     //set server B sockaddr_in
//     serverBaddr.sin_family = AF_INET;
//     serverBaddr.sin_port = htons(serverB_UDP_PORT);
//     serverBaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
// }

//receive from client
void do_service(int new_socket){
    char recvbuf[BUFFLEN];
    int recvLen;
    recvLen = recv(new_socket, recvbuf, sizeof(recvbuf), 0);
    if (recvLen == -1){
        perror("Error in receive message");
        exit(1);
    }else if(recvLen == 0){
        cout << "ending connection" << endl;
    }
    //messgae recieved is countryName, message
    vector<string> usermsg = split(recvbuf, ",");
    countryName = usermsg[0];
    userId = usermsg[1];
    //determine which port this info come from
    socklen_t tcpLen = sizeof(TCPclientAddr);
    getsockname(mainSerTCPfd, (struct sockaddr*)&TCPclientAddr, &tcpLen);
    //how to get the client index
    cout << "The Main server has received the request on User" << userId << "in" << countryName << "from" << "client<client ID>" << "using TCP over port" << TCPclientAddr.sin_port << endl;
}

void accept_from_client(){
    int acceptCli;
    socklen_t TCPclientAddrLen = sizeof(TCPclientAddr);
    acceptCli = accept(mainSerTCPfd, (struct sockaddr *)&TCPclientAddr, &TCPclientAddrLen);
    if (acceptCli == -1){
        perror("Error in accepting socket");
        exit(1);
    }

    pid = fork();
    if (pid == -1){
        perror("Error in fork");
    }
    if (pid == 0){
        //child process
        close(mainSerTCPfd);
        do_service(acceptCli);
        exit(0);
    }else{
        close(acceptCli);
    }

}

void send_to_client(string recommendation){
    char usermsg[BUFFLEN];
    strcpy(usermsg, recommendation.c_str());
    int sendLen = send(serverTcpfd, usermsg, sizeof(usermsg), 0);
    if (sendLen == -1){
        perror("Error in sending message to main server");
        exit(1);
    }
    //how to identify client 1 and client 2
    cout << "The Main Server has sent searching result(s) to client using TCP over port" << TCPPORT <<endl;
}



//receive infofrom servers
string receive_from_server(string server_index){
    char recvmsg[BUFFLEN];
    //mainSerUDP.sin_port = htons(serverA_UDP_PORT);
    int revcLen;
    socklen_t fromlen = sizeof(UDPClientAddr);
    revcLen = recvfrom(mainSerUDPfd, recvmsg, BUFFLEN, 0, (struct sockaddr *)&UDPClientAddr, &fromlen);
    if (revcLen == -1){
            perror("Error in receiving message");
            exit(1);
        }
    cout << "is receving?" << endl;
    cout <<"this is recvmsg::" << recvmsg << endl;
    string restring = recvmsg;
    if (restring == "1"){
        flag = "0";
        return "";
    }
    //socklen_t fromlen;
    //这里需要清空recvmsg嘛
    //memset(recvmsg, '0', sizeof(recvmsg));
    countrySetA.insert(recvmsg);
   

    string res = recvmsg;
    cout << "The Main server has received the country list from server" << server_index << "using UDP over port" << UDPPORT <<endl;
    return res;   
}


void send_to_server(int current_port, string serverName, string request){
    char usermsg[BUFFLEN];
    string str;
    if (request == "1"){
        str = "COUNTRYLIST";
    }
    if (request == ""){
        str = "USERMSG" + countryName + ", " + userId;
    }
    
    strcpy(usermsg, str.c_str());
    int sendLen;
    //decide which server to send the message
    //send to server A

    memset(&mainSerUDP, 0, sizeof(mainSerUDP));
    mainSerUDP.sin_family = AF_INET;
    mainSerUDP.sin_port = htons(current_port);
    mainSerUDP.sin_addr.s_addr = inet_addr("127.0.0.1");
    bind(mainSerUDPfd, (struct sockaddr *)&mainSerUDP, sizeof(mainSerUDP));
    if (sendLen = sendto(mainSerUDPfd, usermsg, sizeof(usermsg), 0, (struct sockaddr *)&mainSerUDP, (socklen_t)sizeof(mainSerUDP)) == -1){
        perror("Error in sending to server");
        exit(1);
    }

    if (request == ""){
        cout << "The Main Server has sent request from User" << userId << "to server" << serverName << "using UDP over port" << UDPPORT << endl;
    }
    request = "";
    cout << "has sending..." << endl;
}

void set_country_server(unordered_set<string> countrySetA, unordered_set<string> countrySetB){
    //for all country stored in A server, set value 0;
    cout << "server A " << endl;
    for (unordered_set<string>::iterator i = countrySetA.begin(); i != countrySetA.end(); i++){
        
        country_server[*i] = 0;
        cout << *i << endl;
    }
    
       
    //for all country stored in B server, set value 0;
    for (unordered_set<string>::iterator i = countrySetB.begin(); i != countrySetB.end(); i++){
        cout << "server B " << endl;
        country_server[*i] = 1;
        cout << *i << endl;
    }    
}

// vector<string> split(string str){
//     stringstream ss(str);
//     istream_iterator<string> begin(ss);
//     istream_iterator<string> end;
//     vector<string> res(begin, end);
//     copy(res.begin(), res.end(), ostream_iterator<string>);
//     return res;
// }

int main(int argc, const char * argv[]) {
    //construct graph
    //int port = 12345;
    
    create_UDP_socket();
    //create_TCP_socket();

    cout << "The Main server is up and running."<< endl; 
    
    
    while(flag == "1"){
        //request and receive contryList from A & B
        send_to_server(serverA_UDP_PORT, "A", "1");
        resA = receive_from_server("A");
        // send_to_server(serverB_UDP_PORT, "B", "1");
        // resB = receive_from_server("B");
        
    }
    set_country_server(countrySetA, countrySetB);
        cout << request << endl;
    flag = "1";
    

/*
    while(1){
        //accept a call from client & receive user query from client
        accept_from_client()
        
        //send & receive user message from server A & B
        if (map.find(countryName) != map.end()){
            if (map.at(countryName) == 0){
                cout << countryName << "shows up in server A" << endl;
                send_to_server(serverA_UDP_PORT, "A", "");
                resA = receive_from_server("A");
            }
            if (map.at(countryName) == 1){
                cout << countryName << "shows up in server B" << endl;
                send_to_server(serverB_UDP_PORT, "B", "");
                resB = receive_from_server("B");
            }
        }else{
            cout << countryName << "does not show up in server A&B" << endl;
        }
        
*/    
        

        //receive user message from serverA & B
                    
    //}

    // char recvmsg[BUFFLEN];
    // socklen_t len = sizeof(mainSerUDP);
    // printf("start receiving\n");
    // recvfrom(mainSerUDPfd, recvmsg, BUFFLEN, 0, (struct sockaddr *)&serverAaddr, &len);
    //printf(*recvmsg);

    // for (int i = 0; i < resA.size(); i++){
    //     cout << resA[i] << endl;
    // }
    //printf("%s",recvmsg);
    

    // set_country_server(countrySetA, countrySetB);

    //std::cout << "Hello, World!\n";
    return 0;
}