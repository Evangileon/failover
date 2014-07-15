all:
	g++ -std=c++0x -lpthread -g -D__DEBUG__ masterfail_1.cpp net_util.cpp heartbeat.cpp statusCheck.cpp -o masterfail
