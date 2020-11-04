Read me
a.	Name: Sirui Pang 

b.	Student ID: 1789442518


c.	What you have done in this assignment:
Built up a socket communication system and implemented a recommendation system for a social network application, which can achieve query & recommendation & reply function. In phase 1, all three servers (servermain and server A & server B) are booting up and run and server A & B send their country list to server main using UDP connection. In phase 2, client can ask server main for user information by inputting user’s country and ID using TCP connection. When the server main received these informations, it sends to either A or B according to which server hold the country information. In phase 3, server A or server B analyze network graph of the country that the client is in and find possible friend for her/him and sends it to the main server using UDP connection. In phase 4, main server replies the client with recommendation result getting from server A & server B.
  
d.	What your code files are and what each one of them does:
servermain.cpp: Build TCP socket to communicate with client. (servermain receives user   information from client and send recommendation result to servermain).
                Build UDP socket to communicate with server A & B. (send user information  to server A or server B according to which server hold these information, and receive recommendation result from them.)
    
serverA.cpp: read the country and user information in “data1.txt”
             Analyze user information and find possible friends of the client.
             Build UDP socket to communicate with servermain. 
             (receive request for country list from server main and send country list (country in data1) to server main.)
             (receive user information from server main and send commendation result to server main.)

serverB.cpp: read the country and user information in “data2.txt”
             Analyze user information and find possible friends of the client.
             Build UDP socket to communicate with servermain. 
             (receive request for country list from server main and send country list (country in data2) to server main.)
             (receive user information from server main and send commendation result to server main.)

client.cpp: Build TCP socket to communicate with servermain. (client send user information to server main and receive recommendation result form servermain).

userdata.txt: Help server main to store the received user information and when server main is to send this information to server A & B, server main will extract the information from this text file.

data1.txt: should be in the same folder with these code to run successful since the relative path for data1.txt in serverA is "./data1.txt"

data2.txt: should be in the same folder with these code to run successful since the relative path for data1.txt in serverB is "./data2.txt"

e.	The format of all the messages exchanged. 
client.cpp:     Get user information from command line input and store them in variable. “countryName” & “Id”. 
                Send user information to server main using TCP connection. 
                Receive recommendation result from server main using TCP connection and store in “recvmsg”, then print different result based on the recommendation result from server main. 
   
servermain.cpp: Communicate with server A & server B using UDP connection
                Send request message “COUNTRYLIST” to server A & B for.  country list.
                Receive country list from server A & server B as “countrySetA” and “countrySetB”. 

                Communicate with client using TCP connection & communicate with server using UDP connection.
                Receive queries from client using TCP connection and stored in text file “userdata.txt”
                Get information from “userdata.txt” and store in variables “countryName” and “userId”, then send them send them to server A or server B according to which server hold this information.
                Receive recommendation result from server A & server B and store it in variable “recommendRes” 
                Send the “recommendRes” (recommend information) to client.

serverA.cpp:    Communicate with servermain using UDP connection.
                Read “data1.txt” to get the country and user information and store in a graph called : “ country_graph”.
                When receive “COUNTRYLIST” from servermain, send all countries stored in its graph to the servermain as “country_list”.
                When receive message is not “COUNTRYLIST”, find possible friend for the client and store it in variable “recommend” and send to servermain. 

serverB.cpp:    Communicate with servermain using UDP connection.
                Read “data2.txt” to get the country and user information and store in a graph called : “ country_graph”.
                When receive “COUNTRYLIST” from servermain, send all countries stored in its graph to the servermain as “country_list”.
                When receive message is not “COUNTRYLIST”, find possible friend for the client and store it in variable “recommend” and send to servermain. 

f.	Any idiosyncrasy of your project. It should say under what conditions the project fails, if any.
Has not found any idiosyncrasy in my project if run these files in order “ serverA”, “serverB”, “servermain”, “client”.
	
G. Reused Code
Most parts of socket, including creating sockets, binding the address, setting port and ip address, recv() recvfrom() send() sendto() function..... came
from Beej's guide line and was edited by me, and also mention it in the comment line

           
           
 


