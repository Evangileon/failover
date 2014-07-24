
#include <thread>
#include <semaphore.h>

#include "master_machine.h"
#include "status_check.h"

sem_t status_send_end_sem;
sem_t status_recv_end_sem;

void master_machine() {

	sem_init(&status_send_end_sem, 0, 0);
	//system("/sbin/ifup eth2");
	std::thread statusSend(master_status_send);
	statusSend.detach();

	
}