#include "GUI.h"
#include <unistd.h>
#include <future>

int main()
{
	GUI *g = new GUI(false);
	userdata d;

	message *buffer;
	int *done;
	vector<message> msg;
	string names[4] = {"abhinav", "chitwan", "rohitrango", "sheshu"};
	for(int i=0;i<4;i++) {
		msg.push_back(message());
		strcpy(msg[i].from, names[i].c_str());
		msg[i].timestamp = 0;
	}

	for(int i=4;i<30;i++) {
		msg.push_back(message());
		strcpy(msg[i].from, ((string)("a"+to_string(i))).c_str());
		msg[i].timestamp = 23;
	}

	// msg.push_back(message());
	// strcpy(msg[0].to, "rohitrango");
	// strcpy(msg[0].from, "abhinav");
	// msg[0].timestamp = 1493326262;	
	// strcpy(msg[0].msg, "");


	// msg.push_back(message());
	// strcpy(msg[1].to, "rohitrango");
	// strcpy(msg[1].from, "chitwan");
	// msg[1].timestamp = 1493326262;	
	// strcpy(msg[1].msg, "");

	// msg.push_back(message());
	// strcpy(msg[2].to, "rohitrango");
	// strcpy(msg[2].from, "sheshu");
	// msg[2].timestamp = 1493326252;	
	// strcpy(msg[2].msg, "Hi Rango!");


	// msg.push_back(message());
	// strcpy(msg[3].to, "sheshu");
	// strcpy(msg[3].from, "rohitrango");
	// msg[3].timestamp = 1493326262;	
	// strcpy(msg[3].msg, "Hi Sheshu!\n");

	// message m;
	// time_t timer;
	// time(&timer);
	// strcpy(m.from, "sheshu");
	// strcpy(m.to, "rohitrango");
	// m.timestamp = timer;	
	// strcpy(m.msg, "Bye.\n");

	string err = "";
	d = g->welcomeHomeScreen(err);
	if(d.usr == "" and d.pwd == "") {
		endwin();
		return 0;	
	}


	// // unsigned int microseconds = 1e6;
	// // usleep(microseconds);
	// // // thread second(g->getFromClient, (&m));

	// first.get();
    g->mainLoop(&msg, buffer, done);
	// mvprintw(g->Y()-2, 1, "Username: %s, Password: %s\n",d.usr.c_str(), d.pwd.c_str());
	// mvprintw(g->Y()-3, 1, "%d %d", g->Y(), g->X());
	endwin();
	return 0;
	
}