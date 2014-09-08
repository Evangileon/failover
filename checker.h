/*
 * checker.h
 *
 *  Created on: Sep 4, 2014
 *      Author: evangileon
 */

#ifndef CHECKER_H_
#define CHECKER_H_

#include <thread>

class checker {
private:
	checker();
	void check_loop();

	static checker& instance();
	int status;
	std::thread *checkThread;
	int do_get_status() { return status; }

public:

	virtual ~checker();
	static int get_status();
};

#endif /* CHECKER_H_ */
