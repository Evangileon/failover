#ifndef __STANDBY_MACHINE_H_JUN__
#define __STANDBY_MACHINE_H_JUN__

#include "master_machine.h"
#include "observer.h"

class standby_machine : public observer
{
public:
	standby_machine();
	~standby_machine();
	void status_receive();
    void inject_thread(std::thread &t);
    void join() { standby_thread.join(); }
    int get_machine_retval() { return retval; }
    void update(int flag);

private:
	int status_receive_loop(int wfd);
	std::thread standby_thread;
	int retval;
	int goingToBeTerminated;
};

extern int needStatusSend;

//extern void * status_receive(void *);
//extern int standby_machine(pthread_t *thread_id);

extern std::shared_ptr<standby_machine> init_standby_machine();


#endif