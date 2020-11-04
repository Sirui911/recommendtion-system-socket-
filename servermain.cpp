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
string flag1 = "1";
string flag2 = "1";
string recommendRes;
int client_index = 0;

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

//get data from "userdata/txt"
vector<string> getData(){
    vector<string> res;
    string filename = "userData.txt";
    ifstream file(filename);
    if (file.is_open()){
        string line;
        while(getline(file, line)){
            res = split(line, ",");
        }
    }
    return res;
    
}

//initiate UDP socket
void create_UDP_socket(){
    //-----------1. create UDP-------------------------
    //reference of this part is Beej guidance
    if ((mainSerUDPfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        perror("Cannot create UDP connection");
        exit(1);
    }
    
    
    memset(&mainSerUDP, 0, sizeof(mainSerUDP));
    mainSerUDP.sin_family = AF_INET;
    mainSerUDP.sin_port = htons(UDPPORT);
    mainSerUDP.sin_addr.s_addr = inet_addr("127.0.0.1");


    // //-----------2. bind UDP----------------------------
    if (bind(mainSerUDPfd, (struct sockaddr *)&mainSerUDP, sizeof(mainSerUDP)) < 0){
        close(mainSerUDPfd);
        perror("cannot bind the UDP socket");
        exit(1);
    }

}

//reference in this part is Beej guidance
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
    mainSerTCP.sin_port = htons(33518);
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



//reference of snedto is Beej guidance
void send_to_server(int current_port, string serverName, string request){
    char usermsg[BUFFLEN];
    string str;
    if (request == "1"){
        str = "COUNTRYLIST";
    }
    if (request == ""){
        str = countryName + "," + userId;
    }
    
    strcpy(usermsg, str.c_str());
    int sendLen;

    //decide which server to send the message
    memset(&mainSerUDP, 0, sizeof(mainSerUDP));
    mainSerUDP.sin_family = AF_INET;
    mainSerUDP.sin_port = htons(current_port);
    mainSerUDP.sin_addr.s_addr = inet_addr("127.0.0.1");
    bind(mainSerUDPfd, (struct sockaddr *)&mainSerUDP, sizeof(mainSerUDP));
    if (sendLen = sendto(mainSerUDPfd, usermsg, sizeof(usermsg), 0, (struct sockaddr *)&mainSerUDP, (socklen_t)sizeof(mainSerUDP)) == -1){
        perror("Error in sending to server");
        exit(1);
    }
    memset(usermsg, '0', sizeof(usermsg));
    if (request == ""){
        cout << "The Main Server has sent request from User" << userId << "to server" << serverName << "using UDP over port" << UDPPORT << endl;
    }
    request = "";
}

//reference of snedto is Beej guidance
void send_to_server2(int current_port, string serverName, string request){
    char usermsg[BUFFLEN];
    string str;
    if (request == "1"){
        str = "COUNTRYLIST";
    }
    if (request == ""){
        str = countryName + "," + userId;
    }
    
    strcpy(usermsg, str.c_str());
    int sendLen;
    //decide which server to send the message

    if ((mainSerUDPfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        perror("Cannot create UDP connection");
        exit(1);
    }


    memset(&mainSerUDP, 0, sizeof(mainSerUDP));
    mainSerUDP.sin_family = AF_INET;
    mainSerUDP.sin_port = htons(current_port);
    mainSerUDP.sin_addr.s_addr = inet_addr("127.0.0.1");
    bind(mainSerUDPfd, (struct sockaddr *)&mainSerUDP, sizeof(mainSerUDP));

    if (sendLen = sendto(mainSerUDPfd, usermsg, sizeof(usermsg), 0, (struct sockaddr *)&mainSerUDP, (socklen_t)sizeof(mainSerUDP)) == -1){
        perror("Error in sending to server");
        exit(1);
    }
    memset(usermsg, '0', sizeof(usermsg));
    if (request == ""){
        cout << "The Main Server has sent request from User <" << userId << "> to server <" << serverName << "> using UDP over port:" << UDPPORT << endl;
    }
    request = "";
}

//receive infofrom servers
//reference of recvfrom is Beej guidance
void receive_from_server(string server_index){
    char recvmsg[BUFFLEN];
    int revcLen;
    socklen_t fromlen = sizeof(UDPClientAddr);
    revcLen = recvfrom(mainSerUDPfd, recvmsg, BUFFLEN, 0, (struct sockaddr *)&UDPClientAddr, &fromlen);
    if (revcLen == -1){
            perror("Error in receiving message");
            exit(1);
        }
    
    //if the receive message is country
    if (flag1 == "1"){
        if (server_index == "A"){
            string restring = recvmsg;
            //if reach the end of the country list, jump out of the receive loop
            if (restring == "NOTHING"){
                flag1 = "0";
                return;
            }

            countrySetA.insert(recvmsg);  
            memset(recvmsg, '0', sizeof(recvmsg));      
        }

        if (server_index == "B"){
            string restring = recvmsg;
            //if reach the end of the country list, jump out of the receive loop
            if (restring == "NOTHING"){
                flag1 = "0";
                return;
            }

            countrySetB.insert(recvmsg);  
            memset(recvmsg, '0', sizeof(recvmsg)); 
        }
    }
        
    
    //if the receive message is the recommend user
    if (flag1 == "0"){
        //there are four case of received message and process them in the client side
        //1. has no recommendation
        //2. connect to every other nodes
        //3. recommendation + client_index
        //4. user not exist
        recommendRes = recvmsg;
        //if the user is not exist
        if (recommendRes == "USER NOT FOUND"){
            cout << "The Main server has received User ID:NOT fount from server <" << server_index <<">" << endl;
        }
        cout << "The Main server has received searching result(s) of User " << userId << "from server " << server_index << endl;
    }
     
}

//reference of sned is Beej guidance
void send_to_client(string recommendation,int new_socket){
    char usermsg[BUFFLEN];
    //if the user is not found or is not in this country
    if (recommendation == "USER NOT FOUND" || recommendation == "None this country"){
        strcpy(usermsg, recommendation.c_str());
        //cout << "usermsge ::" << usermsg << endl;
        int sendLen = send(new_socket, usermsg, sizeof(usermsg), 0);
            if (sendLen == -1){
            perror("Error in sending message to client");
            exit(1);
            }

    }else{
        //has the valid recommendation
        string str = recommendation +","+ to_string(client_index);
        strcpy(usermsg, str.c_str());
        int sendLen = send(new_socket, usermsg, sizeof(usermsg), 0);
            if (sendLen == -1){
            perror("Error in sending message to client");
            exit(1);
            }
    }

    

    if (recommendation == "USER NOT FOUND"){
        cout << "The Main Server has sent error to client using TCP <" << TCPclientAddr.sin_port << ">" <<endl;
    }
    socklen_t tcpLen = sizeof(TCPclientAddr);
    getsockname(new_socket, (struct sockaddr*)&TCPclientAddr, &tcpLen);
    cout << "The Main Server has sent searching result to client using TCP over port:" << TCPclientAddr.sin_port <<endl;
}


//receive from client
//reference of ercv is Beej guidance
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
    getsockname(new_socket, (struct sockaddr*)&TCPclientAddr, &tcpLen);
    string filename = "userData";
    filename.append(".txt");
    ofstream file(filename);
    if (file.is_open()){
        file << countryName << ", " << userId <<endl;
        file.close();
    }
    cout << "The Main server has received the request on User: " << userId << "in " << countryName << " from " << "client <" << client_index  << "> using TCP over port:" << TCPclientAddr.sin_port << endl;

    vector<string> data = getData();
        countryName = data[0];
        userId = data[1];
        
        //send & receive user message from server A & B
        if (country_server.find(countryName) != country_server.end()){
            //the user is in A
            if (country_server.at(countryName) == 0){
                cout << countryName << "shows up in server A" << endl;
                send_to_server2(serverA_UDP_PORT, "A", "");
                receive_from_server("A");
                send_to_client(recommendRes,new_socket);
            }
            //the user is in B
            if (country_server.at(countryName) == 1){
                cout << countryName << "shows up in server B" << endl;
                send_to_server2(serverB_UDP_PORT, "B", "");
                receive_from_server("B");
                send_to_client(recommendRes,new_socket);
            }
        }else{
            //the user does not show up in the server A & B
            cout << countryName << " does not show up in server A & B." << endl;
            recommendRes = "None this country";
            send_to_client(recommendRes,new_socket);
        }
}

void accept_from_client(){
    int acceptCli;
    socklen_t TCPclientAddrLen = sizeof(TCPclientAddr);
    acceptCli = accept(mainSerTCPfd, (struct sockaddr *)&TCPclientAddr, &TCPclientAddrLen);
    if (acceptCli == -1){
        perror("Error in accepting socket");
        exit(1);
    }
    client_index = client_index % 2;
    client_index++;
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







void set_country_server(unordered_set<string> countrySetA, unordered_set<string> countrySetB){
    //for all country stored in A server, set value 0;
    cout << "server A " << endl;
    for (unordered_set<string>::iterator i = countrySetA.begin(); i != countrySetA.end(); i++){
        
        country_server[*i] = 0;
        cout << *i << endl;
    }
    
       
    //for all country stored in B server, set value 0;
    cout << "server B " << endl;
    for (unordered_set<string>::iterator i = countrySetB.begin(); i != countrySetB.end(); i++){
        country_server[*i] = 1;
        cout << *i << endl;
    }    
}


int main(int argc, const char * argv[]) {
    
    create_UDP_socket();
    create_TCP_socket();

    cout << "The Main server is up and running."<< endl; 
    
    
    while(flag1 == "1"){
        //request and receive contryList from A & B
        send_to_server(serverA_UDP_PORT, "A", "1");
        receive_from_server("A"); 
        
    } 
    cout << "The Main server has received the country list from server A using UDP over port" << UDPPORT <<endl;
    flag1="1";
    close(mainSerUDPfd);
    create_UDP_socket();
    

    while(flag1 == "1"){
        send_to_server(serverB_UDP_PORT, "B", "1");
        receive_from_server("B");
        
    }
    cout << "The Main server has received the country list from server B using UDP over port" << UDPPORT <<endl;
    
    close(mainSerUDPfd);
    
    set_country_server(countrySetA, countrySetB);


    while(1){
        accept_from_client();               
    }
    close(mainSerUDPfd);
    return 0;
}