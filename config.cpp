#include <string>
#include <iostream>
#include <fstream>
#include <utility>
#include <memory>

#include "json/json.h"

#include "util.h"
#include "config.h"


config::config() :
		this_is_master(false), config_doc("./config.json"), ip_heartbeat_send_to(
				"0.0.0.0"), ip_heartbeat_recv("0.0.0.0") {
	heartbeat_direct_link = false;
	status_direct_link = false;
	port_heartbeat_sender = 0;
	port_heartbeat_receiver = 0;
	port_status_sender = 0;
	port_status_receiver = 0;
	connect_nonblock_timeout = 5;
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

	this_is_master = root.get("this_is_master", false).asBool();
	heartbeat_direct_link = root.get("heartbeat_direct_link", false).asBool();
	status_direct_link = root.get("status_direct_link", false).asBool();

	// IP address
	// heartbeat related
	ip_heartbeat_send_to = root.get("ip_heartbeat_send_to",
			"192.168.100.101").asString();
	ip_heartbeat_recv = root.get("ip_heartbeat_receive",
			"192.168.100.102").asString();
	
	// status related
	ip_master_status_send_to =
			root.get("ip_master_status_send_to", "10.176.15.200").asString();
	ip_standby_status_recv = root.get("ip_standby_status_receive",
			"10.176.15.201").asString();

	// consistency related
	socket_failover_path = root.get("socket_failover_path",
			"/var/run/failover/failover.ctl").asString();
	pid_failover_path = root.get("pid_failover_path",
			"/var/run/failover/failover.pid").asString();

	// port related
	port_status_sender = root.get("port_status_sender", 44443).asInt();
	port_status_receiver = root.get("port_status_receiver", 44444).asInt();
	port_heartbeat_sender = root.get("port_heartbeat_sender", 44441).asInt();
	port_heartbeat_receiver = root.get("port_heartbeat_receiver", 44442).asInt();

	connect_nonblock_timeout = root.get("connect_nonblock_timeout", 5).asUInt();
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
