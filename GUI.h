/*
Main class for GUI functions
This class will initialize an ncurses window, handle signals and more.
*/
#include <form.h>
#include <ncurses.h>
#include <string>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <vector>
#include <unordered_map>
#include <utility>
#include <signal.h>
#include <menu.h>
#include <time.h>
#include <assert.h>

#define USER_LEN 30
#define MSG_LEN 500
using namespace std;

// CITE: Taken from https://alan-mushi.github.io/2014/11/30/ncurses-forms.html
static char* trim_whitespaces(char *str)
{
	char *end;

	// trim leading space
	while(isspace(*str))
		str++;

	if(*str == 0) // all spaces?
		return str;

	// trim trailing space
	end = str + strnlen(str, MSG_LEN) - 1;

	while(end > str && isspace(*end))
		end--;

	// write new null terminator
	*(end+1) = '\0';

	return str;
}

struct userdata {
	string usr;
	string pwd;

	userdata() {
		usr = "";
		pwd = "";
	}
};

struct message {
	char to[USER_LEN];
	char from[USER_LEN];
	time_t timestamp;
	char msg[MSG_LEN];
	char type;
	; 
	// i for information exchange(success or ffail) ,
	// m for standard message exchange , 
	// t for timestamp packet(timestamp would be 0 if online)
	// l for login 
	// f 
	// r -> start (ready)
};

class GUI {
	private: 
		char myusername[USER_LEN];
		bool debug;
		string talking_to;
		// used to store the messages in real-time according to user.
		// along with the last seen.
		vector<string> usernames;
		unordered_map<string, message> user_last_seen;
		unordered_map<string, pair<vector<message>, int> > chatBox;
		message *buffer;
		int *msg_written;
		WINDOW *up_win;
		WINDOW *border;
		WINDOW *hidden;
		WINDOW *down_win;
		int loop;

		int msg_y, msg_x;
		int chat_y, chat_x, chat_y_start;

		// to keep the chat window.
		int end_line;
		vector<string> chatLine;
		vector<bool> bolded; 

	public:
		GUI(bool debug);
		~GUI();

		int Y();
		int X();

		void errorMSG(string err, int yy, int xx);
		userdata welcomeHomeScreen(string err);		/* home screen */
		void mainLoop(vector<message> *_reg_users, message* &buf, int* &_msg_written);	/* this is the main loop, where the client sends messages */
		string usersMenu();
		void chatScreen(string sel_user);
		void putToDisplay();
		void initDisplay(message *m);
		void getFromClient(message *m);
		void refreshChatWindow();
};
