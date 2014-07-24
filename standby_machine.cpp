
#include <thread>
#include <iostream>
#include "standby_machine.h"
#include "status_check.h"


void standby_machine() {
	system("/sbin/ifdown eth2");
	system("/etc/init.d/asterisk stop");
	
	std::cout << "starting the threads" << std::endl;
	//std::thread receive(receiveMessage);
	//receive.join();
	std::thread statusRecv(status_receive);
	statusRecv.detach();
}