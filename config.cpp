#include <string>
#include <iostream>
#include <fstream>
#include <utility>
#include <memory>

#include "json/json.h"

#include "util.h"
#include "config.h"



#define readInt(name, dfalt) name = root.get(#name, (dfalt)).asInt()
#define readUInt(name, dfalt) name = root.get(#name, (dfalt)).asUInt()
#define readString(name, dfalt) name = root.get(#name, (dfalt)).asString()
#define readBool(name, dfalt) name = root.get(#name, (dfalt)).asBool()

config::config() :
		this_is_master(false), config_doc("/etc/failover/config.json"), ip_heartbeat_receiver(
				"0.0.0.0"), ip_heartbeat_sender("0.0.0.0") {
	heartbeat_direct_link = false;
	status_direct_link = false;
	port_heartbeat_sender = 0;
	port_heartbeat_receiver = 0;
	port_status_sender = 0;
	port_status_receiver = 0;
	connect_nonblock_timeout = 5;
	status_receive_loop_timeout = 0;
}

void config::dump_config_file(std::string dump) const {
	std::ofstream out(dump);
	Json::StyledStreamWriter writer("\t");

	writer.write(out, this->root);
}

void config::parse() {
	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;
	std::ifstream config_file(config_doc);
	bool parsingSuccessful = reader.parse(config_file, root);
	if (!parsingSuccessful) {
		// report to the user the failure and their locations in the document.
		std::cout << "Failed to parse configuration\n"
				<< reader.getFormattedErrorMessages();
		CUR_ERR();
		return;
	}
	this->root = root;

	readBool(this_is_master, false);
	readBool(heartbeat_direct_link, false);
	readBool(status_direct_link, false);

	// IP address
	// heartbeat related
	readString(ip_heartbeat_receiver, "192.168.100.101");
	readString(ip_heartbeat_sender, "192.168.100.102");

	// status related
	readString(ip_master_status_receiver, "10.176.15.200");
	readString(ip_standby_status_sender, "10.176.15.201");

	// consistency related
	readString(socket_failover_path, "/var/run/failover/failover.ctl");
	readString(pid_failover_path, "/var/run/failover/failover.pid");

	// port related
	readInt(port_status_sender, 44443);
	readInt(port_status_receiver, 44444);
	readInt(port_heartbeat_sender, 44441);
	readInt(port_heartbeat_receiver, 44442);

	readUInt(connect_nonblock_timeout, 5);

	Json::Value take_over = root["take_over_script"];
	if (take_over.isArray()) {
		for (Json::ValueIterator itor = take_over.begin();
				itor != take_over.end(); ++itor) {
			take_over_script.push_back((*itor).asString());
		}
	}

	Json::Value fail_over = root["fail_over_script"];
	if (fail_over.isArray()) {
		for (Json::ValueIterator itor = fail_over.begin();
				itor != fail_over.end(); ++itor) {
			fail_over_script.push_back((*itor).asString());
		}
	}

	readUInt(max_restart_times, 3);
	readUInt(status_send_loop_interval, 1);
	readUInt(status_receive_loop_timeout, 2);
	readUInt(status_receive_timeout, 5);

	readUInt(status_send_loop_timeout, 5);
	readUInt(heartbeat_send_loop_timeout, 5);
	readUInt(heartbeat_receive_loop_timeout, 2);
	readUInt(heartbeat_send_interval, 1);

	readString(status_check_script, "/usr/sbin/ProcessChecker.sh");
	readString(network_check_script, "/usr/sbin/NetworkingChecker.sh");
	readUInt(check_interval, 1);

	readString(shell_interpreter, "/bin/bash");

}

void config::update(int flag) {
	if (flag == 0) {
		return;
	}

	/**
	 * ToDo: synchronization issue
	 */
	//std::swap(ip_master_status_send_to, ip_standby_status_recv);
}

std::shared_ptr<config> config::shared() {
	std::shared_ptr<config> px(this, null_deleter());
	return px;
}
