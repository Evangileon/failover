/*
 * signal_handler.h
 *
 *  Created on: Sep 2, 2014
 *      Author: evangileon
 */

#ifndef __SIGNAL_HANDLER_H_JUN__
#define __SIGNAL_HANDLER_H_JUN__

extern struct sigaction old_act;
typedef void (*custom_sa_handler)(int);

extern void setup_signal_handler();

#endif /* __SIGNAL_HANDLER_H_JUN__ */
