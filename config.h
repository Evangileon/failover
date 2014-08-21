#ifndef __CONFIG_H_JUN__
#define __CONFIG_H_JUN__

#include <string>


#define RECEIVE_ONCE_TIMEOUT 5
#define RECEIVE_TIMEOUT 5


#define SERVER_ADDR "10.176.15.103"
#define SLEEP_DUR 5
#define RECV_BUF_SIZE 256
#define MAX_ERR_COUNT 100
#define MAX_DEFUNCT_COUNT 100
#define MAX_CONN_COUNT 50
#define TIMEOUT_DUR 15

#define HEARTBEAT_SEND_PORT 44446

#define STANDBY_ADDR SERVER_ADDR

#define SEND_BUFFER_SIZE 64

#define MAX_RESTART_TIMES 3


class config {
public:
	bool this_is_master;

	std::string config_doc;

    std::string ip_master_heartbeat_send;
    std::string ip_master_heartbeat_recv;
    std::string ip_standby_heartbeat_send;
    std::string ip_standby_heartbeat_recv;

    std::string ip_master_status_send;
    std::string ip_standby_status_recv;

    int port_status_send;
    int port_status_recv;
    int port_heartbeat_send;
    int port_heartbeat_recv;

    std::string socket_failover_path;
    std::string pid_failover_path;

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
