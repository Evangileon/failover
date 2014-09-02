#ifndef __MASTER_MACHINE_H_JUN__
#define __MASTER_MACHINE_H_JUN__

#include <semaphore.h>
#include <pthread.h>

#include <memory>
#include <thread>

#include "observer.h"


class master_machine : public observer {
public:
    master_machine();
    ~master_machine();

    //int master_machine(pthread_t *thread_id);
    void master_status_send();
    void inject_thread(std::thread &t);
    void join() { master_thread.join(); }
    int get_machine_retval() { return retval; }
    void update(int flag);

private:
	int master_status_send_loop(int wfd);
	std::thread master_thread;
	int retval;
    int terminationFlag;
};


extern std::shared_ptr<master_machine> init_master_machine();

#endif
