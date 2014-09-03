#ifndef __MACHINE_INTERACTION_H__
#define __MACHINE_INTERACTION_H__

#include <pthread.h>

#define MASTER_ASTERISK_FINE 1
#define MASTER_ASTERISK_STOP 2
#define MASTER_ALIVE 3
#define MASTER_FAIL 4
#define STANDBY_ALIVE 5
#define STANDBY_FAIL 6

#define ASTERISK_START 7
#define ASTERISK_RESTART 8
#define ASTERISK_STOP 9   // Asterisk running on this machine stopped
#define MACHINE_STOP 10   // The other machine stopped

struct master_status_mtx {
	int isMaster;
	pthread_mutex_t mutex;
};

class thread_terminator {
	pthread_t thread_id;
public:
	thread_terminator();
	thread_terminator(pthread_t id);
	~thread_terminator();
	int terminate(); 
	void set_thread_id(pthread_t id);
};

extern int init_machine_as(struct master_status_mtx *mas_sta, int isMaster);
extern int yield_master(struct master_status_mtx *mas_sta);
extern int is_this_master(struct master_status_mtx *mas_sta);

extern int other_is_master();
extern int seize_master(struct master_status_mtx *mas_sta);

#define init_as_master(mas_sta) init_machine_as((mas_sta), 1)
#define init_as_standby(mas_sta) init_machine_as((mas_sta), 0)

#endif
