#ifndef __HEARTBEAT_H_JUN__
#define __HEARTBEAT_H_JUN__

#include <memory>
#include <thread>
#include <vector>

#include "observer.h"

class heartbeat : public subject {
public:
    heartbeat();
    virtual ~heartbeat();

    void start_heartbeat_send();
    void start_heartbeat_recv();
    void heartbeat_receive();
    void heartbeat_send();

    void set_status_send_id(int id) { status_send_thread_id = id; }
    void set_status_recv_id(int id) { status_recv_thread_id = id; }

    void attach_observer(std::shared_ptr<observer> obs) { views.push_back(obs); }
    void notify_observers(int flag);
    void reset_observer() { views.clear(); }

private:
	int heartbeat_receive_loop(int rfd);
	int heartbeat_send_loop(int wfd);

	int needSend;
	pthread_t status_send_thread_id;
	pthread_t status_recv_thread_id;
	std::thread::id heartbeat_send_thread_id;
	std::thread::id heartbeat_recv_thread_id;

    std::vector<std::shared_ptr<observer> > views;

};

extern std::shared_ptr<heartbeat> init_heartbeat();


#if 0
extern void heartbeat_receive();
extern void heartbeat_send();

extern int start_heartbeat_send();
extern int start_heartbeat_recv();
#endif

#endif
