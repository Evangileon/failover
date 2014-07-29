#ifndef __MASTER_MACHINE_H_JUN__
#define __MASTER_MACHINE_H_JUN__

#include <semaphore.h>

#define MASTER_ASTERISK_STOP 2

extern sem_t status_send_end_sem;
extern sem_t status_recv_end_sem;

extern void* master_status_send(void *);
extern int master_machine();


#endif