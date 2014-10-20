/*
 * script_handler.cpp
 *
 *  Created on: Oct 19, 2014
 *      Author: evangileon
 */

#include "script_handler.h"

script_handler::script_handler() {
	// TODO Auto-generated constructor stub
}

script_handler::script_handler(const std::string& cpath) : path(cpath) {
}

script_handler::~script_handler() {

}

std::string script_handler::get_full_command() {
	std::string full_cmd(path);
	for(std::vector<std::string>::iterator itor = args.begin();
			itor != args.end(); itor++) {
		full_cmd += " ";
		full_cmd += *itor;
	}
	return full_cmd;
}

script_handler::script_handler(const script_handler& handler) {
	path = handler.get_path();
	args = handler.get_args();
}

void script_handler::append_argument(const std::string& arg) {
	args.push_back(arg);
}

script_handler& script_handler::operator =(const script_handler& other) {
	path = other.get_path();
	args = other.get_args();
	return *this;
}
