
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <cstdlib>

#include "config.h"
#include "util.h"
#include "net_util.h"
#include "status_check.h"

int checkStatus() {
    int status = std::system("/bin/bash /usr/sbin/ProcessChecker.sh");
    if (status < 0) {
        ERROR("Can not exec ProcessChecker\n");
    }
    return WEXITSTATUS(status);
}

int checkNetworking() {
    int status = std::system("/bin/bash /usr/sbin/NetworkingChecker.sh");
    if (status < 0) {
        ERROR("Can not exec NetworkingChecker.sh\n");
    }
    return WEXITSTATUS(status);
}

/**
 * Get the string representation of the status result
 * @param  counter index of status
 * @return string representation of the status
 */
std::string checkResultStr(int counter) {
    std::string whatTosend;
    if (counter < 0) {
        ERROR("Can not exec ProcessChecker\n");
    }

    if (counter > 0) { //application is down or not fully functional
        std::cout << "Entering process down section" << std::endl;
        whatTosend = "down";
    } else {
        std::cout << "Everything is fine" << std::endl;
        whatTosend = "ok";
    }

    return whatTosend;
}
