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

#define serverA_UDP_PORT 30518  //UDP PORT for server A
#define main_server_PORT 32518  //UDP PORT for main server
#define BUFFLEN 1000 //length for socket stream buffer


struct sockaddr_in mainServerAddr;
struct sockaddr_in serverAaddr;
int serverAfd;
char buffer[BUFFLEN];
string flag = "NOTHING";//flag to determine whether the country list reach the end


char recvbuf[BUFFLEN];
string country;//store user's country
string user; //store user's ID
string request; //flag to determine that message to send
int exe_flag = 1; //if exe_flag == 0, print the on screen message that all countries have sent to mainserver

//struct to store graph information
struct graph{
    unordered_map<string, unordered_set<string>> user_graph;
    unordered_set<string> user;
    
    void constructGraph();
};

unordered_map<string, graph> contry_graph;//the map to differentiate which graph to use
vector<string> country_list;//store all the countries

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


string findMostDegree(unordered_map<string, unordered_set<string>> userGraph, unordered_set<string> neighbors){
    int max = 0;
    string userIndex = "";
    for(unordered_set<string>::iterator it=neighbors.begin() ;it!=neighbors.end();it++){
        string cur = *it;
        unordered_set<string> curNeighbor = userGraph[cur];
        if (curNeighbor.size() > max){
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
            notConnect.push_back(cur);
        }
     }

     //find common of each two node(node, notconnect)
    int max = 0;
    for (vector<string>::iterator it = notConnect.begin(); it != notConnect.end(); it++){
        int common = 0;
        string candidate = *it;
        unordered_set<string> canNeighbor = userGraph[candidate];
        
        for(unordered_set<string>::iterator it=canNeighbor.begin() ;it!=canNeighbor.end();it++){
            if (neighbors.find(*it) != neighbors.end()){
                common++;
            }
        }
    
        if (common > max){
            userIndex = candidate;
            max = common;
        }
        if (common == max && userIndex > candidate){
            userIndex = candidate;
        }
         }
    return userIndex;

}

//there are four cases in recommendation system
string recommend_to_user(string country, string userId){
    string res = "";
    //find country graph using country_gaph
    graph countryGraph = contry_graph[country];
    //find userGraph corresponding to this country     
    unordered_map<string, unordered_set<string>> userGraph = contry_graph[country].user_graph;
    //find total users
    unordered_set<string> totalUser = countryGraph.user;
    //find neighbors of this client
    unordered_set<string> neighbors = countryGraph.user_graph[userId];
    
    if (totalUser.find(userId) == totalUser.end()){
        return "USER NOT FOUND";
    }

    //case 1:only have one node in this country or all users connect with him/her
    if (totalUser.size() == 1){
        return "No connection";
    }

    //case 2: has connect to every in this country
    if (neighbors.size() == totalUser.size() - 1){
        return "has connect every one";
    }

    //case 3: many nodes in this country, but node has no neighbor
    if (totalUser.size() > 1 && neighbors.size() == 0){
        return findMostDegree(userGraph, totalUser);
    }

    //case 4: many nodes in this country, and node has neighbors
    return findMostCommon(userGraph, neighbors, totalUser, userId);
    

}

//reference of sendto is Beej guidance
void sendToMainServer(){
    int sendLen;
    //i the request is "1", then send country list to the main server
    if (request == "1"){
        char countrymsg[BUFFLEN];
        
        for (vector<string>::iterator item = country_list.begin(); item != country_list.end(); item++){
            string country = *item;
            strcpy(countrymsg, country.c_str());

            if (sendLen = sendto(serverAfd, countrymsg, sizeof(countrymsg), 0, (struct sockaddr *) &mainServerAddr, (socklen_t)sizeof(mainServerAddr)) == -1){
                perror("Error in sending message to main server");
                exit(1);
            }
            memset(countrymsg, '\0', sizeof(countrymsg));
        }
    
        char msg[BUFFLEN];
        strcpy(msg, flag.c_str());
        sendto(serverAfd, msg , sizeof(msg), 0, (struct sockaddr *) &mainServerAddr, (socklen_t)sizeof(mainServerAddr));
        if (exe_flag == 1){
            cout << "The server A has sent a country list to main server" << endl;
            exe_flag = 0;
        }
        
    }
    
    //if request is 0, then the server A should send user recommendation to the mainserver
    if (request == ""){
        cout << "The server A is searching possible friends for User << user << …" <<endl;

        string recommend = recommend_to_user(country, user);
        if (recommend == "USER NOT FOUND"){
            cout << "User <" << user << ">  does not show up in <" << country << ">" << endl;

            
        }
        char recommendInfo [BUFFLEN];

        strcpy(recommendInfo, recommend.c_str());
        //reference in this part is Beej guidance
        if (sendLen = sendto(serverAfd, recommendInfo, sizeof(recommendInfo), 0, (struct sockaddr *) &mainServerAddr, (socklen_t)sizeof(mainServerAddr)) == -1){
                perror("Error in sending message to main server");
                exit(1);
            }
            memset(recommendInfo, '\0', sizeof(recommendInfo));
        if (recommend == "USER NOT FOUND"){
            cout << "The server A has sent user <" << user << "> not found to main server" << endl;
        }else{
            cout <<"The server A has sent the result(s) to Main Server" << "and the result is:" << recommend << endl ;
        }
        


    }
    request = "";
}


//receive User information from main server
void receiveFromMainServer(){
    //receive country
    socklen_t fromlen = sizeof(mainServerAddr);
    int reCountryLen = recvfrom(serverAfd, recvbuf, BUFFLEN, 0, (struct sockaddr *)&mainServerAddr, &fromlen);
    if (reCountryLen < 1){
        perror("Error in receiving country from main server");
        exit(EXIT_FAILURE);
    }
        
    //handle the receive messgae
    string recvmsg = recvbuf;
    //if the received message isa request of he countrylist, then send the country list
    if (recvmsg == "COUNTRYLIST"){
        //set a falg to request, if it is 1, serverA send countryLIst to the mainserver
        request = "1";
    }

    //if the request message is not the countrylist, then send the user message
    if (recvmsg != "COUNTRYLIST"){
        vector<string> userdata = split(recvmsg,",");
        country = userdata[0];
        user = userdata[1];
        country.erase(0, country.find_first_not_of(" "));
        user.erase(0, user.find_first_not_of(" "));
        cout << "The server A has received request for finding possible friends of User"<< user << "in" << country  <<endl;
    }    
}


void contructGraph(){
    //use adjacency list to store the graph
    string curcoun;
    graph curGraph;
    ifstream infile("./data1.txt");
    if (infile.is_open()){
        string line = "";
        while (getline( infile, line )){
            if (!isdigit(line.at(0))){
                country_list.push_back(line);
                curcoun = line;
                continue;
            }else{
            unordered_set<string> neighbors;
            vector<string> lists;
            
            std::string word;

                for ( std::istringstream is( line ); is >> word; )
                {
                    lists.push_back(word);

                }
            string user = lists.at(0);
            contry_graph[curcoun].user.insert(user);
            for (int i = 1; i < lists.size(); i++){
                neighbors.insert(lists.at(i));
            }
            contry_graph[curcoun].user_graph[user] = neighbors;
            }
        }
    }
}
int main(int argc, const char * argv[]) {
    //reference of this socket part is Beej guidance   
    //create_UDP_socket();
    serverAfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&serverAaddr, '\0', sizeof(serverAaddr));

    serverAaddr.sin_family = AF_INET;
    serverAaddr.sin_port = htons(serverA_UDP_PORT);
    serverAaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    //bind UDP socket
    bind(serverAfd, (struct sockaddr *)&serverAaddr, sizeof(serverAaddr));

    cout << "The server A is up and running using UDP on port" << serverA_UDP_PORT << endl; 
    contructGraph();
    
    
    while(1){
        receiveFromMainServer();
        sendToMainServer();
    }
    
    close(serverAfd);
    return 0;
}