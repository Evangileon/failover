#include <fstream>
#include <iostream>
#include <thread>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>

#include <memory>

#include "util.h"
#include "net_util.h"
#include "heartbeat.h"
#include "signal_handler.h"
#include "master_machine.h"
#include "standby_machine.h"
#include "machine_interaction.h"
#include "async_handle_asterisk.h"
#include "create_pid_file.h"
#include "config.h"

int create_socket() {

	int sfd = -1;
	struct sockaddr_un addr;

	sfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sfd == -1) {
		perror("socket");
		exit(-1);
	}

	int reuse = 1;
	if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {
		perror("Can NOT set address to reuseable");
		exit(-1);
	}

	memset(&addr, 0, sizeof(struct sockaddr_un));
	/* Clear structure */
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path,
			config::instance().get_socket_failover_path().c_str(),
			sizeof(addr.sun_path) - 1);

	if ((bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)))
			< 0) {
		if (errno == EADDRINUSE) {
			printf("Failover is already in running\n");
		}
		perror("Can not bind on unix socket");
		exit(-1);
	}

	//std::ofstream pidFile(config::instance().get_pid_failover_path().c_str());
	//pidFile << getpid();
	//pidFile.close();

	return sfd;
}

/**
 *
 * @return -1 if any of fail over script fail
 */
int yield_master_jobs() {
	int ret = 0;
	async_handle_asterisk::stop();

	std::vector<script_handler> fail_over =
			config::instance().get_fail_over_script();
	for (std::vector<script_handler>::iterator itor = fail_over.begin();
			itor != fail_over.end(); ++itor) {
		int err = std::system((*itor).get_full_command().c_str());
		ret = (err < 0) ? -1 : ret;
	}
	return ret;
}

/**
 *
 * @return -1 if any of take over script fail
 */
int takeover_master_jobs() {
	int ret = 0;

	std::vector<script_handler> take_over =
			config::instance().get_take_over_script();
	std::ofstream debug("/home/dslab/debug.txt");
	for (std::vector<script_handler>::iterator itor = take_over.begin();
			itor != take_over.end(); ++itor) {
		std::string cmd = (*itor).get_full_command();
		debug << cmd;
		int err = std::system(cmd.c_str());
		ret = (err < 0) ? -1 : ret;
	}
	debug.close();
	async_handle_asterisk::restart();
	return ret;
}

int main(int argc, char const *argv[]) {
	int test = 0;
	if (1 < argc) {
		if (0 == strcmp(argv[1], "-x")) {
			test = 1;
		}
	}

	setup_signal_handler();
	config::instance().parse();

	int pid_fd = create_pid_file(argv[0],
			config::instance().get_pid_failover_path().c_str(), 0);
	if (pid_fd == PROGRAM_ALREADY_RUNNING) {
		std::cout << "Failover already running" << std::endl;
		exit(0);
	}

	if (test) {
		std::cout << "This is a test\n";
		exit(0);
	}

	struct master_status_mtx mas_sta;

	if (config::instance().is_this_is_master()) {
		init_as_master(&mas_sta);
	} else {
		init_as_standby(&mas_sta);
	}

	std::shared_ptr<heartbeat> hb = init_heartbeat();
	std::cout << "heartbeat module initialized" << std::endl;

	int machine_ret = 0;

	config::instance().dump_config_file("dump_config");

	// Infinite loop
	while (1) {
		// first check whether a master is alive

		if (is_this_master(&mas_sta)) {
			// first check whether a master is alive
			std::cout << "check whether a master is alive" << std::endl;
			if (other_is_master()) {
				yield_master(&mas_sta);
				yield_master_jobs();
				continue;
			}

			std::cout << "This is master" << std::endl;
			async_handle_asterisk::restart();

			std::shared_ptr<master_machine> masterMachine = init_master_machine();
			// add machine instance to heartbeat observer
			// It is an event-driven mechanism
			hb->attach_observer(std::dynamic_pointer_cast<observer>(masterMachine));
			hb->attach_observer(
					std::dynamic_pointer_cast<observer>(
							config::instance().shared()));

			// block here
			masterMachine->join();

			machine_ret = masterMachine->get_machine_retval();
			if (machine_ret == MASTER_ASTERISK_STOP) {
				std::cout << "yield master" << std::endl;
				yield_master(&mas_sta);
				yield_master_jobs();
			}

		} else {
			std::cout << "This is standby" << std::endl;
			async_handle_asterisk::stop();

			std::shared_ptr<standby_machine> standbyMachine = init_standby_machine();
			hb->attach_observer(std::dynamic_pointer_cast<observer>(standbyMachine));
			hb->attach_observer(
					std::dynamic_pointer_cast<observer>(
							config::instance().shared()));

			// block here
			standbyMachine->join();

			machine_ret = standbyMachine->get_machine_retval();
			if (machine_ret == MASTER_ASTERISK_STOP) {
				std::cout << "seize master" << std::endl;
				seize_master(&mas_sta);
				takeover_master_jobs();
			}
		}

		// Clear observers
		hb->reset_observer();
	}

	std::cout << "In the end\n";
	//heartbeatSendThread.join();
	//heartbeatReceiveThread.join();
	std::cout << "After join\n";
	unlink(config::instance().get_socket_failover_path().c_str());

	std::remove(config::instance().get_pid_failover_path().c_str());

	return 0;
}
