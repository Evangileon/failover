#ifndef __STANDBY_MACHINE_H_JUN__
#define __STANDBY_MACHINE_H_JUN__

#include "master_machine.h"

extern int needStatusSend;

extern void * status_receive(void *);
extern int standby_machine();
extern int other_is_master();
extern int seize_master(struct master_status_mtx *mas_sta);

#endif