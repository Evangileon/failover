#ifndef __STATUSCHECK_H_JUN__
#define __STATUSCHECK_H_JUN__

#include <string>

#define CHECK_ASTERISK_FINE 0
#define CHECK_ASTERISK_DUPLICATED 1
#define CHECK_ASTERISK_DEAD 2
#define CHECK_ASTERISK_SHUT_DOWN 3

extern int checkStatus();
extern std::string checkResultStr(int check);

#endif
