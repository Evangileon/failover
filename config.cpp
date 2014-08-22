
#include <string>
#include <iostream>
#include <fstream>
#include <utility>

#include "json/json.h"

#include "util.h"
#include "config.h"

config::config() {
	config_doc = "./config.json";
}


void config::parse() {
    Json::Value root;   // will contains the root value after parsing.
    Json::Reader reader;
    std::ifstream config_file(config_doc);
    bool parsingSuccessful = reader.parse( config_file, root );
    if ( !parsingSuccessful ) {
        // report to the user the failure and their locations in the document.
        std::cout  << "Failed to parse configuration\n"
                   << reader.getFormattedErrorMessages();
        CUR_ERR();
        return;
    }

    this_is_master =
        root.get("this_is_master", false).asBool();

    // IP address
    // heartbeat related
    ip_master_heartbeat_send =
        root.get("ip_master_heartbeat_send", "192.168.100.101").asString();
    ip_master_heartbeat_recv =
        root.get("ip_master_heartbeat_receive", "192.168.100.102").asString();
    ip_standby_heartbeat_send =
        root.get("ip_standby_heartbeat_send", "192.168.100.201").asString();
    ip_standby_heartbeat_recv =
        root.get("ip_standby_heartbeat_receive", "192.168.100.202").asString();
    // status related
    ip_master_status_send =
        root.get("ip_master_status_send", "10.176.15.200").asString();
    ip_standby_status_recv =
        root.get("ip_standby_status_receive", "10.176.15.201").asString();

    // consistency related
    socket_failover_path =
        root.get("socket_failover_path", "/var/run/failover/failover.ctl").asString();
    pid_failover_path =
        root.get("pid_failover_path", "/var/run/failover/failover.pid").asString();

    // port related
    port_status_send =
        root.get("port_status_send", 44444).asInt();
    port_status_recv =
        root.get("port_status_receive", 44444).asInt();
    port_heartbeat_send =
        root.get("port_heartbeat_send", 44445).asInt();
    port_heartbeat_recv =
        root.get("port_heartbeat_receive", 44446).asInt();
}

void config::update(int flag) {
	if(flag == 0) {
		return;
	}

	/// ToDo: synchronization issue
	std::swap(ip_master_status_send, ip_standby_status_recv);
}