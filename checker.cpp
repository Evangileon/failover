/*
 * checker.cpp
 *
 *  Created on: Sep 4, 2014
 *      Author: evangileon
 */

#include <unistd.h>

#include "checker.h"
#include "status_check.h"

checker::checker() {
	// TODO Auto-generated constructor stub
	status = checkStatus();
	checkThread = new std::thread(&checker::check_loop, this);
	checkThread->detach();
}

checker& checker::instance() {
	static checker _instance;
	return _instance;
}

checker::~checker() {
	// TODO Auto-generated destructor stub
}

void checker::check_loop() {
	while(1) {
		sleep(1);
		int check = checkStatus();
		status = check;
	}
}

int checker::get_status() {
	return instance().do_get_status();
}
