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

#define serverB_UDP_PORT 31518  //UDP PORT for server A
#define main_server_PORT 32518  //UDP PORT for main server
#define BUFFLEN 1000 //length for socket stream buffer


struct sockaddr_in mainServerAddr;
struct sockaddr_in serverBaddr;
int serverBfd;
char buffer[BUFFLEN];
socklen_t addrsize;
string flag = "NOTHING";


char recvbuf[BUFFLEN];
string country;
string user;
string request;
int exe_flag = 1;


struct graph{
    unordered_map<string, unordered_set<string>> user_graph;
    unordered_set<string> user;
    
    void constructGraph();
};

unordered_map<string, graph> contry_graph;
vector<string> country_list;

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

//create UDP connection with main server
void create_UDP_socket(){
    //-----------1. create UDP-------------------------
    //int serverAfd = 0;
    
    if (serverBfd = socket(AF_INET, SOCK_DGRAM, 0) == -1){
        perror("Cannot create UDP connection");
        exit(EXIT_FAILURE);
    }
    //serverAfd = socket(AF_INET, SOCK_DGRAM, 0)
    memset(&serverBaddr, '\0', sizeof(serverBaddr));
    // serverAfd = socket(AF_INET, SOCK_DGRAM, 0);
    serverBaddr.sin_family = AF_INET;
    serverBaddr.sin_port = htons(serverB_UDP_PORT);
    serverBaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    // bind(serverAfd, (struct sockaddr *)&serverAaddr, sizeof(serverAaddr));
    //-----------2. bind UDP----------------------------
    // if (bind(serverAfd, (struct sockaddr *)&serverAaddr, sizeof(serverAaddr)) == -1){
    //     perror("cannot bind the UDP socket");
    //     exit(EXIT_FAILURE);
    //     cout<<"1"<<endl;
    // }

}



string findMostDegree(unordered_map<string, unordered_set<string>> userGraph, unordered_set<string> neighbors){
    int max = 0;
    string userIndex = "";
    for(unordered_set<string>::iterator it=neighbors.begin() ;it!=neighbors.end();it++){
        string cur = *it;
        //cout << "cur is ::" << cur << endl;
        unordered_set<string> curNeighbor = userGraph[cur];
        if (curNeighbor.size() > max){
            //cout << "curNeighbor.size():" << curNeighbor.size() << endl;
            userIndex = cur;
            max = curNeighbor.size();
        }
        if (curNeighbor.size() == max || userIndex > cur){
            userIndex = cur;
        }
    }
    return userIndex;      
}

string findMostCommon(unordered_map<string, unordered_set<string>> userGraph, unordered_set<string> neighbors, unordered_set<string> totalUser, string userId){
    //find all nodes not connecting to the ndoe
    string userIndex = "";
    vector<string> notConnect;
     for(unordered_set<string>::iterator it=totalUser.begin() ;it!=totalUser.end();it++){
         string cur = *it;
             if (neighbors.find(cur) != neighbors.end() || cur == userId){
            continue; 
        }else{
            //cout << "cur is " << cur << endl;
            notConnect.push_back(cur);
        }
     }

     //find common of each two node(node, notconnect)
    int max = 0;
    //cout << "notConnect is " << notConnect.size() << endl;
    for (vector<string>::iterator it = notConnect.begin(); it != notConnect.end(); it++){
        int common = 0;
        string candidate = *it;
        //cout << "candidate" << candidate << endl;
        unordered_set<string> canNeighbor = userGraph[candidate];
        //cout << "canNeighbor :: " << canNeighbor.size()<< endl;
        for(unordered_set<string>::iterator it=canNeighbor.begin() ;it!=canNeighbor.end();it++){
            if (neighbors.find(*it) != neighbors.end()){
                common++;
            }
        }
        //cout << "current common is ::" << common << endl;
        if (common > max){
            userIndex = candidate;
            max = common;
        }
        if (common == max && userIndex > candidate){
            userIndex = candidate;
        }
         }
    //cout << "common is " << max << endl;
    return userIndex;

}

//推荐系统分为3种情况
string recommend_to_user(string country, string userId){
    string res = "";
    //find country graph using country_gaph
    graph countryGraph = contry_graph[country];
        
    unordered_map<string, unordered_set<string>> userGraph = contry_graph[country].user_graph;
    //cout << "userGraph size is " << userGraph.size() << endl;
    unordered_set<string> totalUser = countryGraph.user;
    //cout << "total user are" << totalUser.size() << endl;
    unordered_set<string> neighbors = countryGraph.user_graph[userId];
    //cout << "neighbot size is" << neighbors.size() << endl;
    
    if (totalUser.find(userId) == totalUser.end()){
        return "USER NOT FOUND";
    }
    //case 1:only have one node in this country or all users connect with him/her
    if (totalUser.size() == 1){
        return "No connection";
    }
    if (neighbors.size() == totalUser.size() - 1){
        return "has connect every one";
    }

    //case 2: many nodes in this country, but node has no neighbor
    if (totalUser.size() > 1 && neighbors.size() == 0){
        return findMostDegree(userGraph, totalUser);
    }
        //case 3: many nodes in this country, and node has neighbors
    return findMostCommon(userGraph, neighbors, totalUser, userId);
    

}

void sendToMainServer(){
    int sendLen;
    //i the request is "1", then send country list to the main server
    //use sprintf to transfer vector<sring> to string
    if (request == "1"){
        char countrymsg[BUFFLEN];
        
        for (vector<string>::iterator item = country_list.begin(); item != country_list.end(); item++){
            string country = *item;
            strcpy(countrymsg, country.c_str());
            //cout << "send message is countrymsg" << countrymsg << endl;
            //sprintf(contrymsg,"%s", country_list[i]);
            //cout << "this is "<<countrymsg << endl;
            

            if (sendLen = sendto(serverBfd, countrymsg, sizeof(countrymsg), 0, (struct sockaddr *) &mainServerAddr, (socklen_t)sizeof(mainServerAddr)) == -1){
                perror("Error in sending message to main server");
                exit(1);
            }
            memset(countrymsg, '\0', sizeof(countrymsg));
        }
        //request = "";
        //cout << "has sent.." << endl;
        char msg[BUFFLEN];
        strcpy(msg, flag.c_str());
        sendto(serverBfd, msg , sizeof(msg), 0, (struct sockaddr *) &mainServerAddr, (socklen_t)sizeof(mainServerAddr));
        if (exe_flag == 1){
            cout << "The server B has sent a country list to main server" << endl;
            exe_flag = 0;
        }
        
    }
    
    //if request is 0, then the server A should send user recommendation to the mainserver
    if (request == ""){

        cout << "The server B is searching possible friends for User <" << user << ">" <<endl;

        string recommend = recommend_to_user(country, user);
        //cout << "recommend info is :" << recommend << endl;
        if (recommend == "USER NOT FOUND"){
            cout << "User <" << user << ">  does not show up in <" << country << ">" << endl;

            
        }

        char recommendInfo [BUFFLEN];

        strcpy(recommendInfo, recommend.c_str());
        if (sendLen = sendto(serverBfd, recommendInfo, sizeof(recommendInfo), 0, (struct sockaddr *) &mainServerAddr, (socklen_t)sizeof(mainServerAddr)) == -1){
                perror("Error in sending message to main server");
                exit(1);
            }
            memset(recommendInfo, '\0', sizeof(recommendInfo));
          if (recommend == "USER NOT FOUND"){
            cout << "The server B has sent user <" << user << "> not found to main server" << endl;
          }else{
            cout <<"The server B has sent the result(s) to Main Server" << "and the result is:" << recommend << endl ;
          }


    }
    request = "";
}


//receive User information from main server
void receiveFromMainServer(){
    //receive country
    // serverAaddr.sin_port = htons(serverA_UDP_PORT);
    socklen_t fromlen = sizeof(mainServerAddr);
    //cout << "is receiving" << endl;
    int reCountryLen = recvfrom(serverBfd, recvbuf, BUFFLEN, 0, (struct sockaddr *)&mainServerAddr, &fromlen);
    //cout << "recv message is :" << recvbuf << endl;
    if (reCountryLen < 1){
        perror("Error in receiving country from main server");
        exit(EXIT_FAILURE);
    }
        
    //handle the receive messgae
    string recvmsg = recvbuf;
    //cout << "recvmsg" << recvmsg << endl;
    if (recvmsg == "COUNTRYLIST"){
        //set a falg to request, if it is 1, serverA send countryLIst to the mainserver
        request = "1";
    }
    if (recvmsg != "COUNTRYLIST"){
        //cout << "not countrylist" << endl;
        vector<string> userdata = split(recvmsg,",");
        country = userdata[0];
        //cout << "country is " << country << endl;
        user = userdata[1];
        user.erase(0, user.find_first_not_of(" "));
        //cout << "user is " << user << endl;
        cout << "The server B has received request for finding possible friends of User"<< user << "in" << country  <<endl;
        //sendToMainServer();
    }    
}







void contructGraph(){
    //int graphIndex = 0;
    string curcoun;
    graph curGraph;
    ifstream infile("/home/student/Documents/ee450_socket/testcases/testcase2/data2.txt");
    if (infile.is_open()){
        string line = "";
        while (getline( infile, line )){
            if (!isdigit(line.at(0))){
                //cout<<line<<endl;
                country_list.push_back(line);
                curcoun = line;
                //cout << "current country ::" <<curcoun << endl;
                //curGraph = contry_graph[curcoun];
                //counter = counter + 1;
                //cout << "counter is ："<<counter<< endl;
                continue;
            }else{
                  //cout<<line<<endl;
            //cout<<"this is a linr"<<endl;
            unordered_set<string> neighbors;
            vector<string> lists;
            
            std::string word;

                for ( std::istringstream is( line ); is >> word; )
                {
                    //std::cout <<"word" << word << '\n';
                    lists.push_back(word);

                }
            string user = lists.at(0);
            //cout<<"user"<<user<<endl;
            //cout << "line is ::::" << line << endl;
            contry_graph[curcoun].user.insert(user);
            //cout << "curGraph.user size :::" << contry_graph[curcoun].user.size() << endl;
            for (int i = 1; i < lists.size(); i++){
                //cout<<lists.at(i)<<endl;
                neighbors.insert(lists.at(i));
            }
            contry_graph[curcoun].user_graph[user] = neighbors;
            }
        }
    }
}
int main(int argc, const char * argv[]) {
    //construct graph

    //create_UDP_socket();

    serverBfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&serverBaddr, '\0', sizeof(serverBaddr));
    //serverAfd = socket(AF_INET, SOCK_DGRAM, 0);
    serverBaddr.sin_family = AF_INET;
    serverBaddr.sin_port = htons(serverB_UDP_PORT);
    serverBaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    bind(serverBfd, (struct sockaddr *)&serverBaddr, sizeof(serverBaddr));

    cout << "The server B is up and running using UDP on port" << serverB_UDP_PORT << endl; 
    contructGraph();
    
    // if (exe_flag == 1){
        // receiveFromMainServer();
        // sendToMainServer();
    //}
    
    // if (exe_flag == 0){
        while(1){
            receiveFromMainServer();
            sendToMainServer();
        }
    //}

    //exe_flag = 0;
    

    //strcpy(countrymsg, "hello world \n");
    
    //for (int i = 0; i < country_list.size(); i++){
        
        
        //cout << "222222" << endl;
        
        //cout << "555555" << endl;
    //}
    //std::cout << "Hello, World!\n";
    return 0;
}