#ifndef __ASYNC_HANDLE_ASTERISK_H_JUN__
#define __ASYNC_HANDLE_ASTERISK_H_JUN__


#define ASTERISK_START 0
#define ASTERISK_RESTART 1
#define ASTERISK_STOP 2

#include <thread>
#include <mutex>
#include <queue>

#include <semaphore.h>

class async_handle_asterisk
{
private:
	static void handle_loop();

	std::thread *handleThread;
	sem_t work_sem;
	std::mutex critical_lock;
	std::queue<int> handle_queue;
	std::mutex queue_lock;

	async_handle_asterisk();
	~async_handle_asterisk();

	static async_handle_asterisk& instance();
	
public:
	static int start();
	static int restart();
	static int stop();
};

#endif