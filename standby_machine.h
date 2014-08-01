#ifndef __STANDBY_MACHINE_H_JUN__
#define __STANDBY_MACHINE_H_JUN__

#include "master_machine.h"

extern int needStatusSend;

extern void * status_receive(void *);
extern int standby_machine(pthread_t *thread_id);


#endif