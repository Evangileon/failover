#ifndef __CONFIG_H_JUN__
#define __CONFIG_H_JUN__

#include <string>

#define IS_MASTER 1

#define FAILOVER_SOCKET_PATH    "/var/run/failover/failover.ctl"
#define FAILOVER_PID_PATH       "/var/run/failover/failover.pid"

#define RECEIVE_ONCE_TIMEOUT 5
#define RECEIVE_TIMEOUT 5

#define PORT 44444
#define PORTNO "44444"
#define SERVER_ADDR "10.176.15.103"
#define SLEEP_DUR 5
#define RECV_BUF_SIZE 256
#define MAX_ERR_COUNT 100
#define MAX_DEFUNCT_COUNT 100
#define MAX_CONN_COUNT 50
#define TIMEOUT_DUR 15
#define HEARTBEAT_RECEIVE_PORT 44445
#define HEARTBEAT_SEND_PORT 44446

#define STANDBY_ADDR SERVER_ADDR
#define STATUS_PORT 44447

#define SEND_BUFFER_SIZE 64

#define MAX_RESTART_TIMES 3


class config {

	std::string config_doc = "./config.json";

    std::string ip_master_heartbeat_send;
    std::string ip_master_heartbeat_recv;
    std::string ip_standby_heartbeat_send;
    std::string ip_standby_heartbeat_recv;

    config();
public:
    ~config() {}
    
    static config& instance() {
    	static config _instance;
    	return _instance;
    }

    void parse();
    void set_config_path(const std::string& json_path) { config_doc = json_path; }
};

#endif
