#ifndef __CONFIG_H_JUN__
#define __CONFIG_H_JUN__

#include <string>
#include <memory>
#include <map>


#define RECEIVE_ONCE_TIMEOUT 5
#define RECEIVE_TIMEOUT 5

#define SLEEP_DUR 5
#define RECV_BUF_SIZE 256
#define MAX_ERR_COUNT 100
#define MAX_DEFUNCT_COUNT 100
#define MAX_CONN_COUNT 50
#define TIMEOUT_DUR 15
#define SEND_BUFFER_SIZE 64
#define MAX_RESTART_TIMES 3

#include "observer.h"

struct null_deleter
{
    void operator()(void const *) const
    {
    }
};


class config : public observer {
private:
	bool this_is_master;

	std::string config_doc;

    //std::string ip_master_heartbeat_send;
    //std::string ip_master_heartbeat_recv;
    std::string ip_heartbeat_send_to;
    std::string ip_heartbeat_recv;
    std::string ip_standby_heartbeat_send;
    std::string ip_standby_heartbeat_recv;

    std::string ip_master_status_send_to;
    std::string ip_standby_status_recv;

    int port_heartbeat_send_to;
    int port_heartbeat_recv;
    int port_status_send_to;
    int port_status_recv;

    std::string socket_failover_path;
    std::string pid_failover_path;

    std::map<uint, std::string> ip_map;  // id 0 is the master

    config();
public:
    ~config() {}
    
    static config& instance() {
    	static config _instance;
    	return _instance;
    }

    const std::string& get_ip_master_status_send_to() const { return ip_master_status_send_to; }
    
    

    void parse();
    void set_config_path(const std::string& json_path) { config_doc = json_path; }

    std::shared_ptr<config> shared();

	const std::string& get_ip_heartbeat_recv() const {
		return ip_heartbeat_recv;
	}

	const std::string& get_ip_heartbeat_send_to() const {
		return ip_heartbeat_send_to;
	}

	const std::string& get_ip_standby_heartbeat_recv() const {
		return ip_standby_heartbeat_recv;
	}

	const std::string& get_ip_standby_heartbeat_send() const {
		return ip_standby_heartbeat_send;
	}

	const std::string& get_ip_standby_status_recv() const {
		return ip_standby_status_recv;
	}

	const std::string& get_pid_failover_path() const {
		return pid_failover_path;
	}

	int get_port_heartbeat_recv() const {
		return port_heartbeat_recv;
	}

	int get_port_status_recv() const {
		return port_status_recv;
	}

	int get_port_status_send_to() const {
		return port_status_send_to;
	}

	const std::string& get_socket_failover_path() const {
		return socket_failover_path;
	}

	bool is_this_is_master() const {
		return this_is_master;
	}

	void set_this_is_master(bool thisIsMaster) {
		this_is_master = thisIsMaster;
	}

	int get_port_heartbeat_send_to() const {
		return port_heartbeat_send_to;
	}

private:
	virtual void update(int flag);
};

#endif
