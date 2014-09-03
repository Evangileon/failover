
#include <thread>
#include <mutex>
#include <iostream>

#include <semaphore.h>
#include <stdio.h>

#include "async_handle_asterisk.h"
#include "machine_interaction.h"
#include "util.h"

void async_handle_asterisk::handle_loop() {
	while(1) {
		sem_wait(&work_sem);
		
		critical_lock.lock();
		
		queue_lock.lock();
		int handle = handle_queue.front();
		handle_queue.pop();
		queue_lock.unlock();

		switch(handle) {
			case ASTERISK_START:
				system("/etc/init.d/asterisk start");
				break;
			case ASTERISK_RESTART:
				system("/etc/init.d/asterisk restart");
				break;
			case ASTERISK_STOP:
				system("/etc/init.d/asterisk stop");
				break;
			default:
				DEBUG("Unknown handle code");
		}

		critical_lock.unlock();
	}
}

async_handle_asterisk::async_handle_asterisk() {
	if (sem_init(&work_sem, 0, 0) < 0) {
		perror("semaphore");
	}
	handleThread = new std::thread(&async_handle_asterisk::handle_loop, this);
	handleThread->detach();
}

async_handle_asterisk::~async_handle_asterisk() {
	
}

async_handle_asterisk& async_handle_asterisk::instance() {
		static async_handle_asterisk _instance;
		return _instance;
	}

int async_handle_asterisk::start() {
	instance().queue_lock.lock();
	instance().handle_queue.push(ASTERISK_START);
	instance().queue_lock.unlock();

	if (sem_post(&instance().work_sem) < 0) {
		perror("Can not post semaphore");
		ERROR("Can not post semaphore\n");
		return -1;
	}
	std::cout << "Asterisk start succeed" << std::endl;
	return 0;
}

int async_handle_asterisk::restart() {
	instance().queue_lock.lock();
	instance().handle_queue.push(ASTERISK_RESTART);
	instance().queue_lock.unlock();

	if (sem_post(&instance().work_sem) < 0) {
		perror("Can not post semaphore");
		ERROR("Can not post semaphore\n");
		return -1;
	}
	std::cout << "Asterisk restart succeed" << std::endl;
	return 0;
}

int async_handle_asterisk::stop() {
	instance().queue_lock.lock();
	instance().handle_queue.push(ASTERISK_STOP);
	instance().queue_lock.unlock();

	if (sem_post(&instance().work_sem) < 0) {
		perror("Can not post semaphore");
		ERROR("Can not post semaphore\n");
		return -1;
	}
	std::cout << "Asterisk stop succeed" << std::endl;
	return 0;
}
