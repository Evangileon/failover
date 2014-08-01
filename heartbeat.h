#ifndef __HEARTBEAT_H_JUN__
#define __HEARTBEAT_H_JUN__

#include <memory>

class heartbeat {
public:
    heartbeat();
    ~heartbeat();

    void start_heartbeat_send();
    void start_heartbeat_recv();
    void heartbeat_receive();
    void heartbeat_send();
private:
	int heartbeat_receive_loop(int rfd);
	int heartbeat_send_loop(int wfd);

	int needSend = 1;
};

extern std::auto_ptr<heartbeat> init_heartbeat();


#if 0
extern void heartbeat_receive();
extern void heartbeat_send();

extern int start_heartbeat_send();
extern int start_heartbeat_recv();
#endif

#endif
