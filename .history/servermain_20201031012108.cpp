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
string flag1 = "1";
string flag2 = "1";
string recommendRes;
int client_index = 0;

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
    memset(usermsg, '0', sizeof(usermsg));
    if (request == ""){
        cout << "The Main Server has sent request from User" << userId << "to server" << serverName << "using UDP over port" << UDPPORT << endl;
    }
    request = "";
}
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
    //send to server A

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
void receive_from_server(string server_index){
    char recvmsg[BUFFLEN];
    int revcLen;
    socklen_t fromlen = sizeof(UDPClientAddr);
    revcLen = recvfrom(mainSerUDPfd, recvmsg, BUFFLEN, 0, (struct sockaddr *)&UDPClientAddr, &fromlen);
    if (revcLen == -1){
            perror("Error in receiving message");
            exit(1);
        }
    //cout <<"this is recvmsg::" << recvmsg << endl;
    
    //if the receive message is country
    if (flag1 == "1"){
    ///if (!isdigit(recvmsg[0])){
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
    // if (flag2 =="1" && flag1 =="0"){
    //     if (server_index == "B"){
    //         string restring = recvmsg;
    //         //if reach the end of the country list, jump out of the receive loop
    //         if (restring == "NOTHING"){
    //             flag2 = "0";
    //             return;
    //         }

    //         countrySetB.insert(recvmsg);  
    //         memset(recvmsg, '0', sizeof(recvmsg)); 
    //         cout << "The Main server has received the country list from server " << server_index << "using UDP over port" << UDPPORT <<endl; 
    //     }

    // }
        
    
    //if the receive message is the recommend user
    if (flag1 == "0"){
    //if (isdigit(recvmsg[0])){
        //there are four case of received message and process them in the client side
        //1. has no recommendation
        //2. connect to every other nodes
        //3. recommendation + client_index
        //4. user not exist
        recommendRes = recvmsg;
        //cout << "recommendRes is ???? " << recommendRes << endl;
        //if the user is not exist
        if (recommendRes == "USER NOT FOUND"){
            cout << "The Main server has received User ID:NOT fount from server <" << server_index <<">" << endl;
        }
        cout << "The Main server has received searching result(s) of User " << userId << "from server " << server_index << endl;
    }
     
}

void send_to_client(string recommendation,int new_socket){
    char usermsg[BUFFLEN];
    //cout << "recommendatio is::" << recommendation << "client_index is ::" << client_index << endl;
    if (recommendation == "USER NOT FOUND" || recommendation == "None this country"){
        strcpy(usermsg, recommendation.c_str());
        //cout << "usermsge ::" << usermsg << endl;
        int sendLen = send(new_socket, usermsg, sizeof(usermsg), 0);
            if (sendLen == -1){
            perror("Error in sending message to client");
            exit(1);
            }

    }else{
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
    //how to identify client 1 and client 2
    cout << "The Main Server has sent searching result to client using TCP over port:" << TCPclientAddr.sin_port <<endl;
}


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
    getsockname(new_socket, (struct sockaddr*)&TCPclientAddr, &tcpLen);
    string filename = "userData";
    filename.append(".txt");
    ofstream file(filename);
    if (file.is_open()){
        file << countryName << ", " << userId <<endl;
        file.close();
    }
    //how to get the client index
    cout << "The Main server has received the request on User: " << userId << "in " << countryName << " from " << "client <" << client_index  << "> using TCP over port:" << TCPclientAddr.sin_port << endl;

    vector<string> data = getData();
        countryName = data[0];
        userId = data[1];
        //cout << "countryname is :" << countryName << "and id is ::" << userId << endl;
        
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
        //cout << request << endl;
    
    unordered_map<string, int>::iterator it;
    (*it).first;
    (*it).second;
    for (unordered_map<string, int>::iterator i = country_server.begin(); i != country_server.end(); i++){
           int server = i -> second;
           string curCountry = i -> first;
        //country_server[*i] = 0;
        //cout << "country is :" << curCountry << "  and server is ::"<< server <<endl;
    }

    while(1){
        //accept a call from client & receive user query from client
        accept_from_client();
        
        //send_to_client(recommendRes);

                    
    }

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