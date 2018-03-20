#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <ctime>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "GUI.h"
#include <vector>
#include <thread>

#define SERVER_IP "127.0.0.1" // main server
#define SERVER_PORT 9034
#define STDIN 0

using namespace std;

message* buffer = new message;
int* msg_written = new int;
userdata user;
int sockfd;

bool authenticate(){
	// printw("q");
	message msg;
	memset(&msg, 0, sizeof(msg));
	// strcpy(msg.to,"");
	// strcpy(msg.from,(user.usr).c_str());
	// msg.timestamp = time(NULL);
	strcpy(msg.msg,(user.usr+':'+user.pwd).c_str());
	msg.type = 'l';
	if(write(sockfd, (message*) &msg, sizeof(message)) <= 0){
		printf("\nConnection failed \n");
		return -1;
	}
	message res;
	if(read(sockfd, (message*) &res, sizeof(message)) <= 0){
		printf("\nConnection failed \n");
		// mvprintw(0,0,res.msg);
		printf("%s\n",res.msg);
		return -1;
	}
	// printf("%s\n",res.msg);
	if(res.type == 'i' && !strcmp(res.msg,"success")) return 1;
	return 0;
}

bool recvd = 0;

vector<message>* syncUsersData(){
	// printf("1\n");
	vector<message>* msgs;
	msgs = new vector<message>;
	message msg;
	memset(&msg,0,sizeof(msg));
	msg.type = 'r';
	if(write(sockfd, (message*) &msg, sizeof(message)) < 0){
		printf("\nConnection failed \n");
		exit(-1);
	}
	do{
		// printf("2\n");
		if(read(sockfd, (message*) &msg, sizeof(message)) < 0){
			printf("\nConnection failed \n");
			exit(-1);
		}
		// printf("%ld\n",msg.timestamp);
		if(msg.type == 't') msgs->push_back(msg);
	}while(msg.type != 'f');
	msg.type = 'r';
	if(write(sockfd, (message*) &msg, sizeof(message)) < 0){
		printf("\nConnection failed \n");
		exit(-1);
	}
	recvd = 1;
	return msgs;
}

GUI *g;

void mainLoop(){
	vector<message>* msgs = syncUsersData();
	g->mainLoop(msgs, buffer, msg_written);
}

void sendMsg(message *m){
	// exit(0);
	
	if(send(sockfd, (message*) m, sizeof(message),0) <= 0){
		g->errorMSG("error in sending", 0, 0);
	}
	// g->errorMSG(m->msg, 0, 0);
	memset(m,0,sizeof(message));
		// strcpy(buffer->to,"-1\0");
		// strcpy(buffer->from,(user.usr+'\0').c_str());
		// buffer->timestamp = time(NULL);
		// string x; cin >> x;
		// strcpy(buffer->msg,(x+'\0').c_str());
		// buffer->type = 'm';
		// if(send(sockfd, (message*) buffer, sizeof(message),0)<=0){
		// 	// error while writing to socket
		// }
}

int main(){
	bool userVal = 0;
	string err = "";
	*msg_written = 0;

	sockaddr_in serv_addr;
	fd_set readfds,master;
	// char lastread[500];
	// char buffer[500];

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("\nSocket creation error \n");
		return -1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));
  
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERVER_PORT);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr)<=0){
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	if(connect(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		printf("\nConnection Failed \n");
		return -1;
	}

	g = new GUI(0);
	while(!userVal){
		user = g->welcomeHomeScreen(err);
		// printf("%s\n",user.usr.c_str());
		// printw("vsb");
		// cin >> user.usr >> user.pwd;
		if(user.usr == ""){
			close(sockfd);
			endwin();
			return 0;
		}
		userVal = authenticate();

		if(!userVal) err = "User not authenticated.";
	}

	// printf("authenticated\n");

	
	thread guiThread(mainLoop);
	// if(*msg_written == -1)
	// for(vector<message>::iterator it = msgs->begin(); it!=msgs->end(); it++){
	// 	printf("%ld\n",it->timestamp);
	// }


	// while(1){

	// }

	FD_ZERO(&master);
	FD_SET(sockfd,&master);

	// thread sendThread(sendMsg);
	// g->errorMSG(to_string(recvd).c_str(),0,0);
	while(!recvd);
	// exit(0);
	// g->errorMSG(to_string(recvd).c_str(),0,0);
	timeval tt;
	
	while(true){	
		tt.tv_sec = 0;
		tt.tv_usec = 500;	
		// exit(0);
		// if(g->close_loop) break;
		readfds = master;
		if(select(sockfd+1,&readfds,NULL,NULL,&tt)<0){
			// printf("12\n");
			// exit(0);
		}
		// g->errorMSG("kwbvke",0,0);
		if(FD_ISSET(sockfd,&readfds)){
			// exit(0);
			message msg;
			if(recv(sockfd, (message*) &msg, sizeof(message), 0)<=0){
				// error while reading from socket
				endwin();
				exit(0);
			}
			// send msg to gui for display
			// if(msg.type == 'm' or msg.type == 't')
			g->getFromClient(&msg);
			memset(&msg,0,sizeof(msg));
			// strcpy(buffer->to,"-1\0");
			// strcpy(buffer->from,(user.usr+'\0').c_str());
			// buffer->timestamp = time(NULL);
			// string x; cin >> x;
			// strcpy(buffer->msg,(x+'\0').c_str());
			// buffer->type = 'm';
			// if(send(sockfd, (message*) buffer, sizeof(message),0)<=0){
			// 	// error while writing to socket
			// }
			// printf(";dsjb;osj\n");
		}
	}
	guiThread.join();
	// sendThread.join();
	close(sockfd);
	endwin();
	return 0;
}