/*
 * checker.cpp
 *
 *  Created on: Sep 4, 2014
 *      Author: evangileon
 */

#include <unistd.h>

#include "checker.h"
#include "status_check.h"
#include "config.h"

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
	unsigned interval = config::instance().get_check_interval();

	while(1) {
		sleep(interval);
		int check = checkStatus();
		status = check;
	}
}

int checker::get_status() {
	return instance().do_get_status();
}
