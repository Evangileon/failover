
#include <thread>
#include <mutex>

#include <semaphore.h>
#include <stdio.h>

#include "async_handle_asterisk.h"
#include "util.h"

void async_handle_asterisk::handle_loop() {
	while(1) {
		sem_wait(&instance().work_sem);
		
		instance().critical_lock.lock();
		
		instance().queue_lock.lock();
		int handle = instance().handle_queue.front();
		instance().handle_queue.pop();
		instance().queue_lock.unlock();

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

		instance().critical_lock.unlock();
	}
}

async_handle_asterisk::async_handle_asterisk() {
	if (sem_init(&instance().work_sem, 0, 0) < 0) {
		perror("semaphore");
	}
	handleThread = new std::thread(&async_handle_asterisk::handle_loop);
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
		return 0;
	}
	return 1;
}

int async_handle_asterisk::restart() {
	instance().queue_lock.lock();
	instance().handle_queue.push(ASTERISK_RESTART);
	instance().queue_lock.unlock();

	if (sem_post(&instance().work_sem) < 0) {
		perror("Can not post semaphore");
		ERROR("Can not post semaphore\n");
		return 0;
	}
	return 1;
}

int async_handle_asterisk::stop() {
	instance().queue_lock.lock();
	instance().handle_queue.push(ASTERISK_STOP);
	instance().queue_lock.unlock();

	if (sem_post(&instance().work_sem) < 0) {
		perror("Can not post semaphore");
		ERROR("Can not post semaphore\n");
		return 0;
	}
	return 1;
}