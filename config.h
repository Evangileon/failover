#ifndef __CONFIG_H_JUN__
#define __CONFIG_H_JUN__

#include <string>
#include <memory>
#include <map>

#define MAX_CONN_COUNT 50
#define SEND_BUFFER_SIZE 64

#include "json/json.h"

#include "observer.h"

struct null_deleter {
	void operator()(void const *) const {
	}
};

class config: public observer {
private:
	Json::Value root;

	bool this_is_master;
	bool heartbeat_direct_link;
	bool status_direct_link;

	std::string config_doc;

	//std::string ip_master_heartbeat_send;
	//std::string ip_master_heartbeat_recv;
	std::string ip_heartbeat_receiver;
	std::string ip_heartbeat_sender;

	std::string ip_master_status_receiver;
	std::string ip_standby_status_sender;

	int port_heartbeat_sender;
	int port_heartbeat_receiver;
	int port_status_sender;
	int port_status_receiver;

	std::string socket_failover_path;
	std::string pid_failover_path;

	unsigned connect_nonblock_timeout;

	std::map<uint, std::string> ip_map;  // id 0 is the master

	std::vector<script_handler> take_over_script;
	std::vector<script_handler> fail_over_script;

	unsigned max_restart_times;
	unsigned status_send_loop_interval;
	unsigned status_receive_timeout;
	unsigned status_receive_loop_timeout;

	//unsigned status_send_timeout;
	unsigned status_send_loop_timeout;
	//unsigned heartbeat_send_timeout;
	unsigned heartbeat_send_loop_timeout;
	unsigned heartbeat_receive_timeout;
	unsigned heartbeat_receive_loop_timeout;
	unsigned heartbeat_send_interval;

	std::string status_check_script;
	std::string network_check_script;
	unsigned check_interval;

	std::string shell_interpreter;

	config();
public:
	~config() {
	}

	static config& instance() {
		static config _instance;
		return _instance;
	}

	void parse();
	void set_config_path(const std::string& json_path) {
		config_doc = json_path;
	}

	std::shared_ptr<config> shared();

	const std::string& get_ip_heartbeat_sender() const {
		return ip_heartbeat_sender;
	}

	const std::string& get_ip_heartbeat_receiver() const {
		return ip_heartbeat_receiver;
	}

	const std::string& get_ip_master_status_receiver() const {
		return ip_master_status_receiver;
	}

	const std::string& get_ip_standby_status_sender() const {
		return ip_standby_status_sender;
	}

	const std::string& get_pid_failover_path() const {
		return pid_failover_path;
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

	int get_connect_nonblock_timeout() const {
		return connect_nonblock_timeout;
	}

	int get_port_heartbeat_receiver() const {
		return port_heartbeat_receiver;
	}

	int get_port_heartbeat_sender() const {
		return port_heartbeat_sender;
	}

	int get_port_status_receiver() const {
		return port_status_receiver;
	}

	int get_port_status_sender() const {
		return port_status_sender;
	}

	bool is_heartbeat_direct_link() const {
		return heartbeat_direct_link;
	}

	bool is_status_direct_link() const {
		return status_direct_link;
	}

	void dump_config_file(std::string) const;

	const std::vector<std::string>& get_fail_over_script() const {
		return fail_over_script;
	}

	const std::vector<std::string>& get_take_over_script() const {
		return take_over_script;
	}

	unsigned get_max_restart_time() const {
		return max_restart_times;
	}

	unsigned get_status_send_loop_interval() const {
		return status_send_loop_interval;
	}

	unsigned get_status_receive_timeout() const {
		return status_receive_timeout;
	}

	unsigned get_status_receive_loop_timeout() const {
		return status_receive_loop_timeout;
	}

	unsigned get_heartbeat_receive_loop_timeout() const {
		return heartbeat_receive_loop_timeout;
	}

	unsigned get_heartbeat_receive_timeout() const {
		return heartbeat_receive_timeout;
	}

	unsigned get_heartbeat_send_interval() const {
		return heartbeat_send_interval;
	}

	unsigned get_heartbeat_send_loop_timeout() const {
		return heartbeat_send_loop_timeout;
	}

	unsigned get_status_send_loop_timeout() const {
		return status_send_loop_timeout;
	}

	const std::string& get_network_check_script() const {
		return network_check_script;
	}

	const std::string& get_status_check_script() const {
		return status_check_script;
	}

	const std::string& get_shell_interpreter() const {
		return shell_interpreter;
	}

	unsigned get_check_interval() const {
		return check_interval;
	}

private:
	virtual void update(int flag);
	void init_script();
};

#endif
