
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include "config.h"
#include "util.h"
#include "net_util.h"
#include "status_check.h"

int checkStatus() {
    int counter = system("ProcessChecker.sh");
    if(counter < 0) {
        ERROR("Can not exec ProcessChecker\n");
    }
    return counter;
}

std::string checkResultStr(int counter) {
    std::string whatTosend;
    if(counter < 0) {
        ERROR("Can not exec ProcessChecker\n");
    }

    if(counter > 0) { //application is down or not fully functional
        std::cout << "Entering process down section" << std::endl;
        whatTosend = "down";
    } else {
        std::cout << "Everything is fine" << std::endl;
        whatTosend="ok";
    }

    return whatTosend;
}