
#include <string>
#include <iostream>

#include "json/json.h"

#include "util.h"
#include "config.h"

config::config() {
	
}


void config::parse() {
    Json::Value root;   // will contains the root value after parsing.
    Json::Reader reader;
    bool parsingSuccessful = reader.parse( config_doc, root );
    if ( !parsingSuccessful ) {
        // report to the user the failure and their locations in the document.
        std::cout  << "Failed to parse configuration\n"
                   << reader.getFormattedErrorMessages();
        CUR_ERR();
        return;
    }

    ip_master_heartbeat_send =
        root.get("ip_master_heartbeat_send", "192.168.100.101").asString();
    ip_master_heartbeat_recv =
        root.get("ip_master_heartbeat_receive", "192.168.100.102").asString();
    ip_standby_heartbeat_send =
        root.get("ip_standby_heartbeat_send", "192.168.100.201").asString();
    ip_standby_heartbeat_recv =
        root.get("ip_standby_heartbeat_receive", "192.168.100.202").asString();


}