#ifndef __MASTER_MACHINE_H_JUN__
#define __MASTER_MACHINE_H_JUN__

#include <semaphore.h>
#include <pthread.h>

#define MASTER_ASTERISK_STOP 2

struct master_status_mtx {
	int isMaster;
	pthread_mutex_t mutex;
};

extern sem_t status_send_end_sem;
extern sem_t status_recv_end_sem;

extern void* master_status_send(void *);
extern int master_machine();
extern inline int init_machine_as(struct master_status_mtx *mas_sta, int isMaster);
extern int yield_master(struct master_status_mtx *mas_sta);
extern int is_this_master(struct master_status_mtx *mas_sta);

#define init_as_master(mas_sta) init_machine_as((mas_sta), 1)
#define init_as_standby(mas_sta) init_machine_as((mas_sta), 0)

#endif