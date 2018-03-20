all:
	g++ client.cpp GUI.cpp -lmenu -lform -lncurses -lldap -pthread -pg -g -std=c++11 -o cl.out
	g++ selectserver.cpp -lldap -o se
clean:
	rm *.o a.out
server: