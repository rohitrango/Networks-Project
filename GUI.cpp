#include "GUI.h"
#define y LINES
#define x COLS
#define error_y LINES/3

void sendMsg(message *m);

// handle SIGINT
void printErrorMessage(int sig) {
	if(sig == SIGINT or sig == SIGTSTP) {
		int yy, xx;
		getyx(stdscr, yy, xx);
		attron(COLOR_PAIR(1));
		mvprintw(LINES-1, 2, "WARN: Please press Esc to quit.");
		attroff(COLOR_PAIR(1));
		move(yy, xx);
		refresh();
	}
}

string formatName(char *name) {
	string s = name;
	while(s.size()!=30)
		s = s+" ";
	return s;
}


// gives date time format
string dateTimeOf(time_t time) {
	struct tm * timeinfo;
	char buffer [80];

	timeinfo = localtime (&time);
	strftime (buffer,80,"%I:%M%p, %h %d",timeinfo);
	string s = buffer;
	return s;
}

string StatusFromTimestamp(time_t time) {
	if(time==0)
		return "Online";
	else if(time==1)
		return "Never Online.";
	else 
		return dateTimeOf(time);
}

// to get the time
string getTime(time_t timestamp) {
	if(timestamp==-1)
		return "Now.";
	else {
		// CITE: http://www.cplusplus.com/reference/ctime/asctime/
		struct tm * timeinfo;
		timeinfo = localtime ( &timestamp );
		return asctime(timeinfo);
	}
}

void GUI::errorMSG(string err, int yy=-1, int xx=-1) {
	// By default, print at the left bottom
	yy = yy==-1?y-1:yy;
	xx = xx==-1?1:xx;
	int y1, x1; 
	getyx(stdscr, y1, x1);
	move(error_y, 0);
	clrtoeol();
	attron(COLOR_PAIR(1));
	mvprintw(yy, xx, err.c_str());
	attroff(COLOR_PAIR(1));
	move(y1, x1);
	refresh();
}


GUI::GUI(bool debug) {
	this->debug = debug;
	initscr();
	start_color();
	keypad(stdscr, TRUE);
	cbreak();
	noecho();

	// inside nowhere
	loop = 0;

	// handle signal handling
	signal(SIGINT, printErrorMessage);
	signal(SIGTSTP, printErrorMessage);

	// error
	init_pair(1, COLOR_RED, COLOR_BLACK);
	// messages
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_YELLOW, COLOR_BLACK);
	init_pair(4, COLOR_BLUE, COLOR_BLACK);
}

int GUI::Y() {
	return y;
}

int GUI::X() {
	return x;
}

userdata GUI::welcomeHomeScreen(string err) {
	/* Get some useful data and clear screen */
	int ch;
	clear();

	FIELD *field[3];
	FORM  *userform;
	char *t_usr, *t_pwd;
	userdata u;

	// height, width, starty, startx, 
	// number of offscreen rows and number of 
	// additional working buffers
	/* init fields */
	/* 
	Give one last NULL field, apparantly that is used to check
	For forms to init, gives segfault otherwise
	*/
	field[0] = new_field(1, 20, y/2, x/2-10, 0, 0);
	field[1] = new_field(1, 20, y/2+2, x/2-10, 0, 0);
	field[2] = NULL;

	/* Set field options */
	set_field_back(field[0], A_UNDERLINE); 	/* Print a line for the option 	*/
	field_opts_off(field[0], O_AUTOSKIP);  	/* Don't go to next field when this */
											/* Field is filled up 		*/
	set_field_back(field[1], A_UNDERLINE); 
	field_opts_off(field[1], O_AUTOSKIP);

	field_opts_off(field[1], O_PUBLIC);

	/* Creating the form */
	/* Print the form first */
	userform = new_form(field);
	post_form(userform);
	refresh();

	/* write the labels and move cursor*/
	mvprintw(y/2, x/2-22, "Username: ");
	mvprintw(y/2+2, x/2-22, "Password: ");
	move(y/2, x/2-10);
	refresh();

	// Print welcome message
	attron(COLOR_PAIR(2));
	string welMSG = "Welcome to our chat system!";
	mvprintw(y/6, x/2-welMSG.size()/2, welMSG.c_str());
	attroff(COLOR_PAIR(2));
	refresh();

	// Custom errors
	if(err!="") {
		errorMSG(err, error_y, x/2-err.size()/2);
	}

	move(y/2, x/2-10);

	// Until pressing enter or esc (esc to exit)
	bool getout = false;
	while(!getout) {
		ch = getch();
		switch(ch) {
			case 9:
			case KEY_DOWN: 
				/* 
				Go to next field
				Then to the last line of the new field
				*/
				form_driver(userform, REQ_NEXT_FIELD);
				form_driver(userform, REQ_END_LINE);
				break;

			case KEY_UP:
				form_driver(userform, REQ_PREV_FIELD);
				form_driver(userform, REQ_END_LINE);
				break;

			case KEY_LEFT:
				form_driver(userform, REQ_PREV_CHAR);
				break;

			case KEY_RIGHT:
				form_driver(userform, REQ_NEXT_CHAR);
				break;

			case '\n':
				if(form_driver(userform, REQ_VALIDATION) == E_OK) {
					// form is valid
					t_usr = trim_whitespaces(field_buffer(field[0], 0));
					t_pwd = trim_whitespaces(field_buffer(field[1], 0));
					string u1(t_usr), u2(t_pwd);
					refresh();

					/* Clear the line, and then print the message*/
					if(u1 == "" or u2 == "") {
						err = "Fields cannot be empty!";
						errorMSG(err, error_y, x/2-err.size()/2);
						break;
					}

					/* Un post form and free the memory */
					unpost_form(userform);
					free_form(userform);
					free_field(field[0]);
					free_field(field[1]); 

					u.usr = u1;
					u.pwd = u2;

					// if this is wrong, then the function will be called anyway
					bzero(myusername, sizeof(myusername));
					strcpy(myusername, u1.c_str());
					return u;
				}
				else
					break;

			case KEY_BACKSPACE:
			case 127:
				form_driver(userform, REQ_DEL_PREV);
				break;

			case KEY_DC:
				form_driver(userform, REQ_DEL_CHAR);
				break;

			// Esc is pressed
			case 27:
				getout = true;
				break;

			/* Handle key presses here*/
			default:
				form_driver(userform, ch);
				break;
		}
	}

	/* Un post form and free the memory */
	unpost_form(userform);
	free_form(userform);
	free_field(field[0]);
	free_field(field[1]); 

	userdata tmpu;
	return tmpu;
}


/*
This main loop will first parse all the users and their last seen.
The user stores the last seen and then parses the messages.
If any new message has higher timestamp, then it has a new message. (Update the bool accordingly)
*/
void GUI::mainLoop(vector<message> *_reg_users, message* &buf, int* &_msg_written) {
	// init buffer and msg bool
	buffer = buf;
	msg_written = _msg_written;
	int tag;

	// Add to last seen
	// Add to last seen vector
	vector<message> reg_users = *(_reg_users);
	for(int i=0; i<reg_users.size(); i++) {
		string other_user = reg_users[i].from;
		if(other_user == myusername)
			continue;
		usernames.push_back(other_user);
		user_last_seen[other_user] = reg_users[i];
		chatBox[other_user] = pair<vector<message>, int>();
		chatBox[other_user].second = 0;
	}

	// Menu will contain details -> name, last seen and if any new messages 
	while(true) {
		string selected_user = usersMenu();
		// mvprintw(y-1,1,"selected user is: %s\n", selected_user.c_str());
		if(selected_user == "") {
			// user wants to quit
			break;
		}
		chatScreen(selected_user);
	}

	msg_written = new int;
	*msg_written = -1;
	// main loop closes means that the user is done.
}

/*
This function will return the user selected from the list.
Open a list of people with their last seen and new messages.
*/
string GUI::usersMenu() {
	// one null item

	loop = 1;
	clear();
	int n = user_last_seen.size();
	int c;
	int sub_y = y-4, sub_x = x-4;
	WINDOW *sub_window = newwin(sub_y, sub_x, 2, 2);
	box(sub_window, 0, 0);
	refresh();
	wrefresh(sub_window);
	keypad(sub_window, true);

	// selected vs highlighted
	int highlighted = 0;
	int selected = 0;

	mvwaddch(sub_window, 2, 0, ACS_LTEE);
	mvwhline(sub_window, 2, 1, ACS_HLINE, x-5);
	mvwaddch(sub_window, 2, x-5, ACS_RTEE);

	// heading
	string h = "List of registered users.";
	wattron(sub_window, A_BOLD);
	mvwprintw(sub_window, 1, 1, "Username");
	mvwprintw(sub_window, 1, x/2 - 7, "Last seen");
	mvwprintw(sub_window, 1, sub_x-15, "# New messages");
	wattron(sub_window, COLOR_PAIR(2));
	mvprintw(1, x/2 - h.size()/2, h.c_str());
	wattroff(sub_window, COLOR_PAIR(2));
	wattroff(sub_window, A_BOLD);
	mvprintw(y-2, 2, "Note: Use the arrow keys to navigate, Enter to chat, Esc to exit.");

	bool getout = false;
	int t_y, t_x;
	int start;
	string sel_user = "";
	while(!getout) {

		if(selected < sub_y-5)
			start = 0;
		else
			start = selected - (sub_y-5);

		// print the users list
		for(int i=0; i<sub_y-4; i++) {
			if(i+start == n)
				break;
			string cur_user_time = StatusFromTimestamp(user_last_seen[usernames[i+start]].timestamp).c_str();
			if(i == highlighted)
				wattron(sub_window, A_REVERSE | COLOR_PAIR(4));
			else if(cur_user_time == "Online")
				wattron(sub_window, COLOR_PAIR(2));
			else 
				wattron(sub_window, COLOR_PAIR(3));
			wmove(sub_window, i+3, 0);
			wclrtoeol(sub_window);
			mvwprintw(sub_window, i+3, 1, usernames[i+start].c_str());
			mvwprintw(sub_window, i+3, x/2 - 7, cur_user_time.c_str());
			// print detail
			mvwprintw(sub_window, i+3, x-12, to_string(chatBox[usernames[i+start]].second).c_str());
			wattroff(sub_window, A_REVERSE | COLOR_PAIR(2) | COLOR_PAIR(3) | COLOR_PAIR(4));
		}

		if(debug) {
			mvprintw(y-2, 1, to_string(highlighted).c_str());
			mvprintw(y-2, x/3, to_string(selected).c_str());
			mvprintw(y-2, 2*x/3, to_string(x).c_str());
		}

		wmove(sub_window, selected, 1);
		move(y-1, 1);
		box(sub_window, 0, 0);
		mvwaddch(sub_window, 2, 0, ACS_LTEE);
		mvwaddch(sub_window, 2, x-5, ACS_RTEE);
		wrefresh(sub_window);

		c = getch();
		switch(c) {

			case KEY_DOWN: 
				if(selected < n-1) {
					selected+=1;
					highlighted = (highlighted==sub_y-5)?highlighted:highlighted+1;
				}
				break;
			case KEY_UP:
				if(selected > 0) {
					selected-=1;
					highlighted = (highlighted!=selected+1)?highlighted:highlighted-1;
				}
				break;
			case '\n':
				sel_user = usernames[selected];
				getout = true;
				break;
			// Esc is pressed
			case 27:
				endwin();
				exit(0);
		}
	}

	delwin(sub_window);
	return sel_user;
}


/* 
This box will check for the selected 
*/
void GUI::chatScreen(string sel_user) {

	loop = 2;
	bolded.resize(0);
	chatLine.resize(0);
	bolded.push_back(false);
	chatLine.push_back("");
	end_line = -1;
	// init
	talking_to = sel_user;
	chatBox[talking_to].second = 0;
	clear();
	// up window : Start from (2,2) -> (msg_y+2, msg_x+2) == (y-5, x-2) 
	// down window : Start from (msg_y+2, 2) -> (msg_y+6, chat_x+2) ==> (y-5, 2) == (y-1, x-2) -> 4,3,2
	chat_y = 6, chat_x = x-4;
	msg_y = y-chat_y-3, msg_x = x-4;
	chat_y_start = msg_y+2;

	sel_user = "Chat for : " + sel_user;
	attron(A_BOLD);
	move(1,1);
	clrtoeol();
	mvprintw(1, x/2 - sel_user.size()/2, sel_user.c_str());
	attroff(A_BOLD);

	border = newwin(msg_y, msg_x, 2, 2);
	down_win = newwin(chat_y, chat_x, msg_y+2, 2); 

	/* CITE: Creating subwindows and writing stuff into it: https://linux.die.net/man/3/subwin */
	// Write all messages into it
	up_win = subwin(border, msg_y-2, msg_x-2, 3, 3);
	hidden = newwin(y, msg_x-2, y, x);
	scrollok(up_win, true);
	scrollok(hidden, true);
	for(int i=0;i<chatBox[talking_to].first.size(); i++) {
		message m = chatBox[talking_to].first[i];
		initDisplay(&m);
	}
	int y_u, x_u;
	getyx(up_win, y_u, x_u);
	wmove(up_win, y_u-1, x_u);
	touchwin(border);
	wrefresh(up_win);

	box(border, 0, 0);
	box(down_win, 0, 0);
	keypad(down_win, true);

	// refresh everything
	refresh();
	wrefresh(border);
	wrefresh(down_win);

	// make a form
	FIELD *tbox[2];
	FORM *form;
	tbox[0] = new_field(chat_y-2, chat_x-2, 1, 1, 0, 0);
	tbox[1] = NULL;
	form = new_form(tbox);

	set_form_win(form, down_win);
	set_form_sub(form, down_win);
	post_form(form);
	wmove(down_win, 1, 1);
	box(down_win, 0, 0);
	move(msg_y+3, 3);
	wrefresh(down_win);

	int ch;
	bool getout = false;
	while(!getout) {
		ch = getch();
		switch(ch) {

			case KEY_LEFT:
				form_driver(form, REQ_PREV_CHAR);
				break;

			case KEY_RIGHT:
				form_driver(form, REQ_NEXT_CHAR);
				break;

			case '\n':
				// CITE: http://stackoverflow.com/questions/18493449/how-to-read-an-incomplete-form-field-ncurses-c
				if(form_driver(form, REQ_VALIDATION) == E_OK) {
					string u1(trim_whitespaces(field_buffer(tbox[0], 0)));
					if(u1 == "")
						break;

					// Copy details
					msg_written = new int;
					*msg_written = 0;
					buffer = new message();
					strcpy(buffer->to, 	 talking_to.c_str());
					strcpy(buffer->from, myusername);
					time(&(buffer->timestamp));
					strcpy(buffer->msg, u1.c_str());
					buffer->type = 'm';
					// Put to display
					putToDisplay();
					*msg_written = 1;

					// clear screen
					form_driver(form, REQ_CLR_FIELD);
					break;
				}
				
			case KEY_BACKSPACE:
			case 127:
				form_driver(form, REQ_DEL_PREV);
				break;

			case KEY_DC:
				form_driver(form, REQ_DEL_CHAR);
				break;

			// Page up 
			case KEY_PPAGE:
				if(end_line >= msg_y-2) {
					end_line--;
					refreshChatWindow();
				}
				touchwin(border);
				wrefresh(up_win);
				break;

			// page down
			case KEY_NPAGE:
				if(end_line < (long int)(chatLine.size()-1)) {
					end_line++;
					refreshChatWindow();
				}
				touchwin(border);
				wrefresh(up_win);
				break;

			// Esc is pressed
			case 27:
				getout = true;
				break;

			/* Handle key presses here*/
			default:
				form_driver(form, ch);
				break;
		}
		refresh();
		wrefresh(down_win);		
	}

	/* Un post form and free the memory */
	unpost_form(form);
	free_form(form);
	free_field(tbox[0]);

	delwin(down_win);
	delwin(up_win);
	delwin(border);
	// talking to no one now
	talking_to = "";

	bolded.resize(0);
	chatLine.resize(0);
	loop = 0;
}

/* 
This is another function which takes the message from STDIN and then prints it to the screen.
Since this is new, its added to the vector as well.
*/
void GUI::putToDisplay() {
	// Try to figure this out
	if(chatBox[talking_to].first.size() == 0) {
		chatBox[talking_to].first.push_back(*buffer);
	}
	message ttt = *(chatBox[talking_to].first.end()-1);
	if(!(ttt.timestamp == buffer->timestamp and !strcmp(ttt.msg, buffer->msg))) {
		chatBox[talking_to].first.push_back(*buffer);
	}
	initDisplay(buffer);
	sendMsg(buffer);
}

/*
This will print the initial stuff on the GUI (from the stored vector), we do not need to add it to the vector
*/
void GUI::initDisplay(message *m) {
	
	string f_from = m->from, f_to = m->to, me = myusername;
	mvprintw(0,0,"%s %s %s", f_from.c_str(), f_to.c_str(), me.c_str());
	refresh();
	move(0,0);
	assert((!strcmp(m->to, myusername) or !strcmp(m->from, myusername)));
	assert((!strcmp(m->from,talking_to.c_str()) or !strcmp(m->to,talking_to.c_str())));
	
	// break the message and add it to buffer
	string header = "@" + (string)(m->from) + "\t" + getTime(m->timestamp);
	string my_msg = (string)(m->msg);
	string tmp;
	int minLine;
	while(header.size()) {
		minLine = min(msg_x-2, (int)header.size());
		tmp = header.substr(0, minLine);
		chatLine.push_back(tmp);
		bolded.push_back(true);
		header = header.substr(minLine);
		end_line++;
	}

	while(my_msg.size()) {
		minLine = min(msg_x-2, (int)my_msg.size());
		tmp = my_msg.substr(0, minLine);
		chatLine.push_back(tmp);
		bolded.push_back(false);
		my_msg = my_msg.substr(minLine);
		end_line++;
	}

	chatLine.push_back("\n");
	bolded.push_back(false);
	end_line++;

	end_line = chatLine.size()-1;
	refreshChatWindow();

	waddch(down_win, ' ');
	wrefresh(down_win);

	touchwin(border);
	wrefresh(up_win);
}

/*
Takes from client and prints on GUI.
Since this will be called by the client, its a new message and MUST be added to the vector
*/
void GUI::getFromClient(message *m) {
	// message from someone else
	if(m->type == 'm') {

		// Try to figure this out
		if(chatBox[talking_to].first.size() == 0) {
			chatBox[talking_to].first.push_back(*buffer);
		}
		message ttt = *(chatBox[talking_to].first.end()-1);
		if(!(ttt.timestamp == buffer->timestamp and !strcmp(ttt.msg, buffer->msg))) {
			chatBox[talking_to].first.push_back(*buffer);
		}


		string f_from = m->from, f_to = m->to, me = myusername;
		assert(f_from == me or f_to == me);
		
		if(f_from != talking_to and f_to != talking_to) {
			
			// Try to figure this out
			if(chatBox[f_from].first.size() == 0) {
				chatBox[f_from].first.push_back(*m);
				chatBox[f_from].second += 1;
			}
			message ttt = *(chatBox[f_from].first.end()-1);
			if(!(ttt.timestamp == m->timestamp and !strcmp(ttt.msg, m->msg))) {
				chatBox[f_from].first.push_back(*m);
				chatBox[f_from].second += 1;
			}

			// mvprintw(y-1, 0, "%s %d %s", f_from.c_str(), chatBox[f_from].first.size(), "returning.");
			// refresh();
			return;
		}
		else {
			// Try to figure this out
			if(chatBox[f_from].first.size() == 0) {
				chatBox[f_from].first.push_back(*m);
			}
			message ttt = *(chatBox[f_from].first.end()-1);
			if(!(ttt.timestamp == m->timestamp and !strcmp(ttt.msg, m->msg))) {
				chatBox[f_from].first.push_back(*m);
			}
			// mvprintw(y-1, 0, "%s %d YYY", f_from.c_str(), chatBox[f_from].first.size());
			// refresh();
		}

		// Do this only inside a chat window
		if(loop == 2) {
			string header = "@" + (string)(m->from) + "\t" + getTime(m->timestamp);
			string my_msg = (string)(m->msg);
			string tmp;
			int minLine;
			while(header.size()) {
				minLine = min(msg_x-2, (int)header.size());
				tmp = header.substr(0, minLine);
				chatLine.push_back(tmp);
				bolded.push_back(true);
				header = header.substr(minLine);
				end_line++;
			}

			while(my_msg.size()) {
				minLine = min(msg_x-2, (int)my_msg.size());
				tmp = my_msg.substr(0, minLine);
				chatLine.push_back(tmp);
				bolded.push_back(false);
				my_msg = my_msg.substr(minLine);
				end_line++;
			}

			chatLine.push_back("\n");
			bolded.push_back(false);
			end_line++;

			end_line = chatLine.size()-1;
			refreshChatWindow();

			touchwin(border);
			wrefresh(up_win);
		}
	}
	else if (m->type == 't') {
		string fromuser = m->from; 
		if(user_last_seen.find(fromuser) == user_last_seen.end()) {
			user_last_seen[fromuser] = *m;
			usernames.push_back(fromuser);
		}
		else {
			user_last_seen[fromuser].timestamp = m->timestamp;
		}
	}
}

// Refresh the window
void GUI::refreshChatWindow() {
	if(loop == 2) {
		wclear(up_win);
		int start = max(end_line-(msg_y-3), 0);
		int index = 0;
		if(debug)
			mvprintw(0 ,0 , "%d %d %d", start, end_line, msg_y-2);
		else
			mvprintw(0 ,2 , "Press PgUp and PgDown to scroll messages, Esc to exit.");
		refresh();
		for(int i=start; i<=end_line; i++) {
			if(bolded[i])
				wattron(up_win, A_BOLD);
			mvwprintw(up_win, index, 0, chatLine[i].c_str());
			wattroff(up_win, A_BOLD);
			index++;
		}

		waddch(down_win, ' ');
		wrefresh(down_win);
		refresh();
	}
}