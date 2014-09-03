/*
 * create_pid_file.h
 *
 *  Created on: Sep 3, 2014
 *      Author: evangileon
 */

#ifndef CREATE_PID_FILE_H_
#define CREATE_PID_FILE_H_

/* create_pid_file.h

   Header file for create_pid_file.c.
*/

#define PROGRAM_ALREADY_RUNNING -2

#define CPF_CLOEXEC 1

extern int create_pid_file(const char *progName, const char *pidFile, int flags);


#endif /* CREATE_PID_FILE_H_ */
