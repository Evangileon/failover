#ifndef __STANDBY_MACHINE_H_JUN__
#define __STANDBY_MACHINE_H_JUN__

extern int needStatusSend;

extern void * status_receive(void *);
extern int standby_machine();

#endif