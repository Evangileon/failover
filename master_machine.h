#ifndef __MASTER_MACHINE_H_JUN__
#define __MASTER_MACHINE_H_JUN__

#include <semaphore.h>

extern sem_t status_send_end_sem;
extern sem_t status_recv_end_sem;

extern void master_status_send();
extern void master_machine();


#endif