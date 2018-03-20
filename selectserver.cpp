/*
** selectserver.c -- a cheezy multiperson chat server
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <map>
#include <queue>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <signal.h>
#include <time.h>
#include <iostream>
#define LDAP_DEPRECATED 1
#include <stdio.h>
#include <ldap.h>
using namespace std;

#define PORT "9034"   // port we're listening on

struct message{
	char to[30];
	char from[30];
	long int timestamp;
	char msg[500];
	char type; //i for information exchange(success or ffail) , m for standard message exchange , t for timestamp packet(timestamp would be 0 if online), l for login credentials client to server
};

struct user{
	int sockfd;
	long int lastseen;	//Only useful when user in not online
	bool online;
	bool ready_ls;
	bool ready_m;
	queue<message> messages;
	user(){	//Default contructor
		sockfd=0;
		lastseen=0;
		online=false;
		ready_ls=false;
		ready_m = false;
	}
};


map<int,string> socketmap;
map<string,user> users;

time_t timer;
int listener;     // listening socket descriptor
fd_set master;    // master file descriptor list
int fdmax;        // maximum file descriptor number
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
void populate_messages(int socketid)
{
			//Empty the to send stack
	cout<<"Stack size is "<<users[socketmap[socketid]].messages.size()<<endl;
	if(users[socketmap[socketid]].messages.empty()){
		return;
	}
	message tosend = users[socketmap[socketid]].messages.front();
	printf("%s\n",tosend.msg);
	printf("%c\n",tosend.type);
	if(send(socketid,(message*)(&tosend),sizeof(tosend),0) == -1){
		perror("send");
	} //This is gold, dig it
	while(!users[socketmap[socketid]].messages.empty()){
		message tosend = users[socketmap[socketid]].messages.front();
		printf("%s\n",tosend.msg);
		printf("%c\n",tosend.type);
		users[socketmap[socketid]].messages.pop();
		if(send(socketid,(message*)(&tosend),sizeof(tosend),0) == -1){
			perror("send");
		}
	}
	users[socketmap[socketid]].ready_m = true;

}
void populate_lastseen(int socketid)
{

	message tosend;
	tosend.type = 't';
	for(map<string,user>::iterator it = users.begin();it != users.end();++it){
		sprintf(tosend.from,"%s",it->first.c_str());
		if(it->second.online == false){
			tosend.timestamp = it->second.lastseen;
		}
		else{
			tosend.timestamp = 0;
		}
		if(send(socketid,(char*)(&tosend),sizeof(tosend),0) == -1){
			perror("send");
		}
	}
	memset(&tosend,0,sizeof(tosend));
	tosend.type = 'f';
	if(send(socketid,(char*)(&tosend),sizeof(tosend),0) == -1){
			perror("send");
		}


	users[socketmap[socketid]].ready_ls = true;
}
bool authenticate(message* credentials,int socketid){
	if(credentials->type!='l'){
		message tosend;
		tosend.type = 'i';
		sprintf(tosend.msg,"fail");
		if(send(socketid,(char*)(&tosend),sizeof(tosend),0)){
			perror("send");
		}
		return false;
	}
	string combo(credentials->msg),password,username;
	if(combo[combo.length()-1]=='\n'){
		combo = combo.substr(0,combo.length()-1);
	}
	cout<<combo<<combo.length();
	size_t foundpos = combo.find(':');
	if(foundpos!=string::npos){
		password = combo.substr(foundpos+1);
		username = combo.substr(0,foundpos);
		cout<<password<<password.length()<<" "<<username.length();
		// exit(0);
		LDAP *ld;
	    int rc;
	    int version = LDAP_VERSION3;

	    if (ldap_initialize (&ld, "ldap://cs252lab.cse.iitb.ac.in:389")) {
	        perror("ldap_init"); /* no error here */
	        return(1);
	    }

	    ldap_set_option (ld, LDAP_OPT_PROTOCOL_VERSION, &version);
	    string s =  "cn="+ username + ",dc=cs252lab,dc=cse,dc=iitb,dc=ac,dc=in";
	    rc = ldap_bind_s(ld,s.c_str(),password.c_str(), LDAP_AUTH_SIMPLE);
		//search in database now
		if(rc == LDAP_SUCCESS){
				socketmap[socketid] = username;
				users[username].online = true;
				users[username].sockfd = socketid;
				message tosend;
				tosend.type = 'i';
				sprintf(tosend.msg,"success");
				if(send(socketid,(char*)(&tosend),sizeof(tosend),0)){
					perror("send");
				}
				// exit(0);
				//List of all users with last seen/ online  and their lastseen
				
				//Send all other users that a new online user has come up
				tosend.type = 't';
				tosend.msg[0] = '\0';
				tosend.to[0] = '\0';
				sprintf(tosend.from,"%s",username.c_str());
				tosend.timestamp = 0;

				for(int j = 0; j <= fdmax; j++) {
					// send to everyone!
					if (FD_ISSET(j, &master)) {
						// except the listener and ourselves
						if (j != listener && j != socketid) {
							if (send(j,(char*)(&tosend),sizeof(tosend),0) == -1) {
								perror("send");
							}
						}
					}
				}
							
				ldap_unbind(ld);
				return true;
		}
		else{
			message tosend;
			tosend.type = 'i';
			sprintf(tosend.msg,"fail");
			if(send(socketid,(char*)(&tosend),sizeof(tosend),0)){
				perror("send");
			}
			ldap_unbind(ld);
			return false;
		}
			
	}
	else{
		message tosend;
		tosend.type = 'i';
		sprintf(tosend.msg,"fail");
		if(send(socketid,(char*)(&tosend),sizeof(tosend),0)){
			perror("send");
		}
		return false;
		//the combo string didn't have a : , which means wrong format 
	}
}

void terminator(int sig){
	char c;
	signal(sig,SIG_IGN);
	printf("Do you want to close the server? [y/n]\n");
	c=getchar();
	if(c=='y'||c=='Y'){
		//Code to be run on termination here
		//last seen updation
	FILE * pFile;
    pFile = fopen ("temp.csv","w");

	for(map<string,user>::iterator it = users.begin();it != users.end();++it){
		while(!it->second.messages.empty())
		{
			message m = it->second.messages.front();
			fprintf(pFile,"%s,%s,%ld,%s\n",m.to,m.from,m.timestamp,m.msg);
			it->second.messages.pop();
		}
	}
	remove("messages.csv");
	rename("temp.csv","messages.csv");
	fclose (pFile);

    pFile = fopen ("temp.csv","w");
    time(&timer);
	for(map<string,user>::iterator it = users.begin();it != users.end();++it){
		if(it->second.online)
		{
			it->second.lastseen = timer;
		}
		fprintf(pFile,"%s,%ld\n",it->first.c_str(),it->second.lastseen);
	}
	remove("lastseen.csv");
	rename("temp.csv","lastseen.csv");
	fclose (pFile);

	//update timers

	exit(0);
	}
	else{
		signal(SIGINT,terminator);
	}
	getchar(); //Get the newline character
}

int main(void)
{

   //

	//Populate the users map from csv
	//No blank line at the end of file
	signal(SIGINT,terminator);
	string line;
	string username;
	ifstream read("lastseen.csv");
	while(getline(read,line)){
		istringstream ss(line);
		string timestamp;
		getline(ss, username, ',');
		getline(ss, timestamp);
		if(username != ""){
			users[username].lastseen=atoi(timestamp.c_str());
		}
	}
	read.close();
	

	//Populate user messages pending from last session
	read.open("messages.csv");
	string to,from,timestamp,messagecontent;
	while(getline(read,line)){
		istringstream ss(line);
		getline(ss, to, ',');
		getline(ss, from, ',');
		getline(ss, timestamp, ',');
		getline(ss, messagecontent);
		if(users.find(to)==users.end()){//<flag1>
			printf("Stop kidding\n");
		}

		else{
			message temp;
			sprintf(temp.to,"%s",to.c_str());
			sprintf(temp.from,"%s",from.c_str());
			sprintf(temp.msg,"%s",messagecontent.c_str());
			temp.timestamp = atoi(timestamp.c_str());
			temp.type = 'm';
			users[to].messages.push(temp);
		}
	}
	read.close();



	fd_set read_fds;  // temp file descriptor list for select()
	int newfd;        // newly accept()ed socket descriptor
	struct sockaddr_storage remoteaddr; // client address
	socklen_t addrlen;

	message buf;    // buffer for client data
	int nbytes;

	char remoteIP[INET6_ADDRSTRLEN];

	int yes=1;        // for setsockopt() SO_REUSEADDR, below
	int i, j, rv;

	struct addrinfo hints, *ai, *p;

	FD_ZERO(&master);    // clear the master and temp sets
	FD_ZERO(&read_fds);

	// get us a socket and bind it
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
		exit(1);
	}
	
	for(p = ai; p != NULL; p = p->ai_next) {
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0) { 
			continue;
		}
		
		// lose the pesky "address already in use" error message
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			close(listener);
			continue;
		}

		break;
	}

	// if we got here, it means we didn't get bound
	if (p == NULL) {
		fprintf(stderr, "selectserver: failed to bind\n");
		exit(2);
	}

	freeaddrinfo(ai); // all done with this

	// listen
	if (listen(listener, 10) == -1) {
		perror("listen");
		exit(3);
	}

	// add the listener to the master set
	FD_SET(listener, &master);

	// keep track of the biggest file descriptor
	fdmax = listener; // so far, it's this one

	// main loop
	for(;;) {
		read_fds = master; // copy it
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(4);
		}

		// run through the existing connections looking for data to read
		for(i = 0; i <= fdmax; i++) {
					printf("%d 2\n",i);
			
			if (FD_ISSET(i, &read_fds)) { // we got one!!
					printf("%d 1\n",i);
				
				if (i == listener) {
					// handle new connections
					printf("%d\n",i);
					addrlen = sizeof remoteaddr;
					newfd = accept(listener,
						(struct sockaddr *)&remoteaddr,
						&addrlen);

					if (newfd == -1) {
						perror("accept");
					} else {
						FD_SET(newfd, &master); // add to master set
						if (newfd > fdmax) {    // keep track of the max
							fdmax = newfd;
						}
						
						printf("selectserver: new connection from %s on "
							"socket %d\n",
							inet_ntop(remoteaddr.ss_family,
								get_in_addr((struct sockaddr*)&remoteaddr),
								remoteIP, INET6_ADDRSTRLEN),
							newfd);

					}
				} else {
					// handle data from a client
					printf("1\n");
					if ((nbytes = recv(i, (message*)&buf, sizeof buf, 0)) <= 0) {
						// got error or connection closed by client
						if (nbytes == 0) {
							// connection closed
							printf("selectserver: socket %d hung up\n", i);
						} else {
							perror("recv");
						}
						close(i); // bye!
						FD_CLR(i, &master); // remove from master set
						users[socketmap[i]].online = false;
						//<flag> timestamp update
						time(&timer);
						users[socketmap[i]].lastseen = timer;
						socketmap.erase(i);

						message tosend;
						tosend.type = 't';
						tosend.msg[0] = '\0';
						tosend.to[0] = '\0';
						sprintf(tosend.from,"%s",socketmap[i].c_str());
						tosend.timestamp = timer;

						for(int j = 0; j <= fdmax; j++) {
							// send to everyone!
							if (FD_ISSET(j, &master)) {
								// except the listener and ourselves
								if (j != listener && j != i) {
									if (send(j,(char*)(&tosend),sizeof(tosend),0) == -1) {
										perror("send");
									}
									printf("Sent to %d\n",j);
								}
							}
						}

					}
					else{
						// we got some data from a client
						// exit(0);
						printf("%c %s\n",buf.type,buf.msg);
						// exit(0);
						// printf("%ld\n",socketmap.size());
						// if(users.find(socketmap[i])!=users.end()){
						// 	printf("ehjwvwevblw\n");
						// 	// printf("%s\n",socketmap[i].c_str());
						// }
						if(socketmap.find(i)==socketmap.end() || socketmap.size()==0){
							printf("auth1\n");
							authenticate((struct message*)(&buf),i);
						}
						else if(users.find(socketmap[i])==users.end()){//<flag1>
							printf("auth2\n");
							authenticate((struct message*)(&buf),i);
						}
						else if(buf.type == 'r' && users[socketmap[i]].ready_ls == false)
						{
							printf("populating lastseen for %d\n", i);
							populate_lastseen(i);
						}
						else if(buf.type == 'r' && users[socketmap[i]].ready_m == false)
						{
							printf("populating message for %d\n", i);
							populate_messages(i);
							printf("populating message for %d\n", i);

						}
						else{
							//Message exchange
							message* tosend;
							tosend = (struct message*)(&buf); //<flag>
							printf("%s\n",tosend->msg);
							if(tosend->type == 'm'){
								cout<<tosend->msg<<endl;
								string temp(tosend->msg);
								if(temp[temp.length()-1]=='\n'){
									temp = temp.substr(0,temp.length()-1);
									bzero(tosend->msg,sizeof(tosend->msg));
									sprintf(tosend->msg,"%s",temp.c_str());
								}
								if(users[tosend->to].online){
									if(send(users[tosend->to].sockfd,(message*)(tosend),sizeof(*tosend),0)){
										perror("send");
									}
								}
								else{
									users[tosend->to].messages.push(*tosend);
								}
							}
						}
					}
				} // END handle data from client
			}
			memset(&buf,0,sizeof(buf));
			 // END got new incoming connection
		} // END looping through file descriptors
	} // END for(;;)--and you thought it would never end!
	
	return 0;

}

