
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


std::string checkResultStr() {
    std::string whatTosend;
    int counter = system("ProcessChecker.sh"); 
    if(counter < 0) {
        ERROR("Can not exec ProcessChecker\n");
    }

    if(counter > 0) { //application is down or not fully functional
        std::cout << "Entering process down section" << std::endl;
                //system("/sbin/ifdown eth2"); // we bring down the interface and the asterisk service on the system
                //system("/etc/init.d/asterisk stop");        
        whatTosend = "down";
                // send the message no action from my side       
                //asFailure=1;
                //isMaster=0;                   
                //sleep(SLEEP_DUR);
    } else {
        std::cout << "Everything is fine" << std::endl;
        whatTosend="ok";
    }

    return whatTosend;
}
