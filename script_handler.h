/*
 * script_handler.h
 *
 *  Created on: Oct 19, 2014
 *      Author: evangileon
 */

#ifndef SCRIPT_HANDLER_H_
#define SCRIPT_HANDLER_H_

#include <string>
#include <vector>

class script_handler {
	std::string path;
	std::vector<std::string> args;

public:
	script_handler();
	script_handler(const std::string& cpath);
	script_handler(const script_handler& handler);

	virtual ~script_handler();
	std::string get_full_command();
	void append_argument(const std::string& arg);

	script_handler& operator= (const script_handler& other);

	const std::vector<std::string>& get_args() const {
		return args;
	}

	const std::string& get_path() const {
		return path;
	}
};

#endif /* SCRIPT_HANDLER_H_ */
