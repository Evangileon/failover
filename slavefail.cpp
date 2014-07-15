#include<fstream>
#include<iostream>
#include<sstream>
#include<stdio.h>
#include<stdlib.h>
#include<limits.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<list>
#include<ctime>
#include<string>
#include<string.h>
#include<time.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include<map>
#include<queue>
#include<cstdatomic>
#include<algorithm>
#include<math.h>
#include<thread>
#include<error.h>
#include<ctime>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <signal.h>

//ports used for communication

#define FAILOVER_SOCKET_PATH    "/var/run/failover/failover.ctl"
#define FAILOVER_PID_PATH       "/var/run/failover/failover.pid"


#define PORT 44444
#define PORTNO "44444"
#define SERVER_ADDR "10.176.15.103"
#define SLEEP_DUR 5
#define RECV_BUF_SIZE 256
#define MAX_ERR_COUNT 100
#define MAX_DEFUNCT_COUNT 100
#define MAX_CONN_COUNT 50
#define TIMEOUT_DUR 15

void receiveMessage();
 
// get1 sockaddr, IPv4 or IPv6:
void *get1_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
 
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int first_time=1;
bool isMaster=0;
/**
 * Initial Role of this machine, distinguish from isMaster, which varies when the HA status change
 */
bool isInitMaster = 0;

int sockfd, numbytes;  
struct addrinfo hints1, *servinf, *q;
int rv;
char s[INET6_ADDRSTRLEN];
bool initial_exec=0;

void sendRequest( char *reqms)
{
    char MyHostName[_POSIX_HOST_NAME_MAX + 1];
    gethostname(MyHostName,sizeof MyHostName);
    std::string myname(MyHostName);

    std::ifstream readsystem("system.in");
    std::string servername;
   
	servername=SERVER_ADDR;
	char *messageTo=new char[servername.length()];
	servername.copy(messageTo, servername.length());
	messageTo[servername.length()]='\0';
    if(first_time==1)
    {
    	first_time=0;
	    memset(&hints1, 0, sizeof hints1);
	    hints1.ai_family = AF_UNSPEC;
	    hints1.ai_socktype = SOCK_STREAM;
	 
	    if ((rv = getaddrinfo(messageTo, PORTNO, &hints1, &servinf)) != 0) {
	       std::cout<<"\nerror getting the addr info"; 
	       return;
	    }
	 
	    // loop through all the results and connect to the first we can
	    for(q = servinf; q != NULL; q = q->ai_next) {
	        if ((sockfd = socket(q->ai_family, q->ai_socktype,
	                q->ai_protocol)) == -1) {
	            std::cout<<"client: socket\n";
	            continue;
	        }
	 
	        if (connect(sockfd, q->ai_addr, q->ai_addrlen) == -1) {
	            close(sockfd);
	            std::cout<<"client: connect error "<<errno<<"\n";
	            continue;
	        }
	 
	        break;
	    }
	 
	    if (q == NULL) 
		{
	        std::cout<<"stderr, client: failed to connect\n";
	        return;
    	}
 
	    inet_ntop(q->ai_family, get1_in_addr((struct sockaddr *)q->ai_addr), s, sizeof s);
	    printf("client: connecting to %s\n", s);
	    //std::cout<<"this server is connecting to the other server, to pass the exit message:"<< s;
 
    	freeaddrinfo(servinf); // all done with this structure
	
	//if i have crashed and trying to come up again
	if(initial_exec== 1 && isMaster == 0)
	{
		initial_exec=0;
		//std::thread receive(receiveMessage);
		//receive.detach();
	}
    }
	
    if((numbytes=send(sockfd,reqms,sizeof(reqms),0)) == -1)
    { 
    	std::cout<<"there was an error on forwarding the request to the other server"<<messageTo<<std::endl;
    	//first_time=1;
    	sleep(SLEEP_DUR);
    }
 	std::cout<<"numbytes "<<numbytes<<"\n";
	bzero(reqms,0);
	return;
}

//we will have 2 threads working always on the system
/* the first one will be to send the messages to the other server and the other thread would be to receive the messages from the other server
*/

int acceptingConn=0;
int stop_recv=0;

void sendMessage()
{ 
	int niError=0;
	int errorStatus=0;
	int counter=0;
	int zombiecount=0;
	int asFailure=0;
	int ethWorking=0;
	
	while(1)
	{
		sleep(SLEEP_DUR);
		while(acceptingConn==1)
		{
			sleep(SLEEP_DUR);
			continue;
		}
		if(isMaster)
		{
		    
		    //check for all eth ports
		    system("ethtool eth0 | grep 'Link detected: no' | wc -l > /tmp/check.log");
		    std::ifstream check1("/tmp/check.log");
		    int isEthDown=0;
		    check1>>isEthDown;
		    check1.close();
		    
		    if(isEthDown==0 && ethWorking==1)
		    {
		    	//reconnect to the end system
                std::cout << "eth0 is down\n";
                first_time=1;//connect to the end system again
		    	isMaster=0;//I am no more the master since i need to send
		    	ethWorking=0;//reset so that this part is executed only once
		    	stop_recv=0;//reset the recv thread since we need to make a new conn to end host
		    	continue;
		    }
		    
		    if(isEthDown==1)
		    {
		    	//eth0 down let the end system take over
		    	system("/sbin/ifdown eth2");
                std::cout << "eth0 is down\n";
		    	system("/etc/init.d/asterisk stop");
                isMaster = 0;
		    	stop_recv=1;
		    	ethWorking=1;
		    	continue;
		    }
		    
		    system("ethtool eth2 | grep 'Link detected: no' | wc -l > /tmp/check.log");
		    std::ifstream check("/tmp/check.log");
		    int getcount(0);
		    check>>getcount;
		    check.close();
		    //std::cout<<getcount;
		
			if(getcount==1)//I am no more the master
			{
		        std::cout<<"entering 1\n" <<std::endl;
		        std::string whatTosend="yes";
		        char *Mess= new char[whatTosend.length()];
			    whatTosend.copy(Mess,whatTosend.length());
			    Mess[whatTosend.length()]='\0';
			    niError=1;
			    isMaster=0;
			    system("/sbin/ifdown eth2");
			    //system("/etc/init.d/asterisk stop");
			    // send the message no action from my side
			    std::thread message1(sendRequest, std::ref(Mess));
			    message1.join();
			    sleep(SLEEP_DUR);
		  	}
		
		    //I need to check my system state
		    //condition 1.. (if I get errors in my /var/log/message repeatedly, I need to migrate the service)
		    system("tail -500 /var/log/messages | grep error | wc -l > /tmp/error.log");
		    std::ifstream checkerror("/tmp/error.log");
		    int howmanyErrors;
		    checkerror>>howmanyErrors;
		    checkerror.close();
		    
		    if(howmanyErrors >=MAX_ERR_COUNT)
		    {
		    	errorStatus++;
		    }
		    //if I get these many errors repeatdely say for twice, i.e. error frequency is high, we migrate the service
		    if(errorStatus ==2 && getcount == 0)
		    {
		    	std::cout<<"entering 3" <<std::endl;
		
		        system("/sbin/ifdown eth2");
		        std::string whatTosend="yes";
		        char *Mess= new char[whatTosend.length( )];
		        whatTosend.copy(Mess,whatTosend.length( ));
		        Mess[whatTosend.length()]='\0';
		
		        // send the message no action from my side
		        std::thread message2(sendRequest, std::ref(Mess));
		        message2.join();
		        sleep(SLEEP_DUR);// we bring down the interface and the asterisk service on the system
		        errorStatus=0;
		    }
		
		    //if the process goes down on the system
			system("ProcessChecker.sh; echo $? > /tmp/processcount.log");       
		    std::ifstream procount("/tmp/processcount.log");
		    procount>>counter;
		    procount.close();
		
		    if( counter!=0 && getcount == 0) {//application is down or not fully functional
		    	std::cout<<"entering process down section" <<std::endl;
		        system("/sbin/ifdown eth2"); // we bring down the interface and the asterisk service on the system
		        system("/etc/init.d/asterisk stop"); 
		        
		        std::string whatTosend="yes";
		        char *Mess= new char[whatTosend.length( )];
		        whatTosend.copy(Mess,whatTosend.length( ));
		        Mess[whatTosend.length()]='\0';
		        // send the message no action from my side
		        std::thread message3(sendRequest, std::ref(Mess));
		        message3.join();
		        asFailure=1;
				isMaster=0;
		    } 
            
            if(counter==0) {
		    	std::string whatTosend="ok";
                std::cout << "This iteration is ok\n";
		        char *Mess= new char[whatTosend.length( )];
		        whatTosend.copy(Mess,whatTosend.length( ));
		        Mess[whatTosend.length()]='\0';
		        // send the message no action from my side
		        std::thread message3(sendRequest, std::ref(Mess));
		        message3.join();
		    }
		    
		    /*if((counter==0) && (asFailure == 1))
		    {
		       std::string whatTosend="give";
		       char *Mess= new char[whatTosend.length())];
		       whatTosend.copy(Mess,whatTosend.length());
		       Mess[whatTosend.length()]='\0';
		       // send the message no action from my side
		       std::thread message3(sendRequest, std::ref(Mess));
		       message3.join();
		       asFailure = 0;
		       sleep(SLEEP_DUR);
		       system("/sbin/ifup eth2");
		       system("service asterisk start");
		    }*/       
		
		    //if the defunct process count goes high
		    system("ps -ef | grep defunct | wc -l > /tmp/defunct.log ");
		    std::ifstream defcount("/tmp/defunct.log"); 
		    defcount >> zombiecount;
		    defcount.close();
		    remove("/tmp/defunct.log");
		    if(zombiecount >MAX_DEFUNCT_COUNT && getcount == 0)
		    {
		        std::cout<<"entering 5 " <<std::endl;
		        system("/sbin/ifdown eth2"); // we bring down the interface and the asterisk service on the system
		        sleep(SLEEP_DUR);
		        std::string whatTosend="yes";
		        char *Mess= new char[whatTosend.length( )];
		        whatTosend.copy(Mess,whatTosend.length( ));
		        Mess[whatTosend.length()]='\0';
		        // send the message no action from my side
		        std::thread message3(sendRequest, std::ref(Mess));
		        message3.join();
		        zombiecount=0;
		    }
		}
		else
		{
			system("ProcessChecker.sh; echo $? > /tmp/processcount.log");       
		    std::ifstream procount("/tmp/processcount.log");
		    procount>>counter;
		    procount.close();
		
		    if(counter!=0)//application is down or not fully functional
		    {
		    	std::cout<<"entering process down section" <<std::endl;
		        //system("/sbin/ifdown eth2"); // we bring down the interface and the asterisk service on the system
		        //system("/etc/init.d/asterisk stop"); 
		        
		        std::string whatTosend="down";
		        char *Mess= new char[whatTosend.length( )];
		        whatTosend.copy(Mess,whatTosend.length( ));
		        Mess[whatTosend.length()]='\0';
		        // send the message no action from my side
		        std::thread message3(sendRequest, std::ref(Mess));
		        message3.join();
		        //asFailure=1;
				//isMaster=0;                   
		        //sleep(SLEEP_DUR);
		    }
		    else
		    {
		    	std::string whatTosend="ok";
		        char *Mess= new char[whatTosend.length( )];
		        whatTosend.copy(Mess,whatTosend.length( ));
		        Mess[whatTosend.length()]='\0';
		        // send the message no action from my side
		        std::thread message3(sendRequest, std::ref(Mess));
		        message3.join();
		    }
		    sleep(SLEEP_DUR);
		}
	}
	std::cout<<"sending thread closed\n";
}

int first_time_recv=0;

void receiveMessage()
{
	//let us setup the receving socket
	int n;
	int sockfdq, portno;
	socklen_t clilen;
	int optval;
	char *buffer=new char[RECV_BUF_SIZE];
	struct timeval tv;
	struct sockaddr_in serv_addr, cli_addr;
	if(first_time_recv==1)
	{
	    char MyHostNamee[_POSIX_HOST_NAME_MAX + 1];
	    gethostname(MyHostNamee,sizeof MyHostNamee);
	
	
	    sockfdq = socket(AF_INET, SOCK_STREAM, 0);
	    if (sockfdq < 0) 
	    std::cout<<"ERROR opening socket\n";
	    bzero((char *) &serv_addr, sizeof(serv_addr));
	    portno = PORT;
	    serv_addr.sin_family = AF_INET;
	    serv_addr.sin_addr.s_addr = INADDR_ANY;
	    serv_addr.sin_port = htons(portno);
	    if (bind(sockfdq, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
	    	std::cout<<"ERROR on binding\n";
	    listen(sockfdq,MAX_CONN_COUNT);
	    int getcount(0);
	 	int fail_count=0;
		//always listening on the port to read the messages
		
		clilen = sizeof(cli_addr);
		sockfd = accept(sockfdq, (struct sockaddr *) &cli_addr, &clilen);
		if (sockfd< 0) 
			std::cout<<"ERROR on accept\n";
		bzero(buffer,RECV_BUF_SIZE);
		close(sockfdq);
		std::thread send(sendMessage);
		send.detach();

		//optval = 1;
		//setsockopt(sockfdq, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
	}
	
	tv.tv_sec = TIMEOUT_DUR;
	tv.tv_usec=0; 
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof tv)) 
	{ 
		perror("setsockopt"); 
	}
	while(1) {
		
        if(!isMaster) {		                             
			n = recv(sockfd,buffer,RECV_BUF_SIZE-1, 0);
			if (n <= 0) {
				if(stop_recv==1)
				{
					//our sys down stop recv
					continue;
				}
				acceptingConn=1;
				std::cout<<"end system not well_1"<<std::endl;
				close(sockfd);
				close(sockfdq);
				//system("service asterisk stop");
				sleep(SLEEP_DUR);
			        //I start the asterisk app and then bring up the ip
				// NOT neccessary right now
                //system("/sbin/ifup eth2");
				system("service asterisk start");
				isMaster=1;
				
				//int sockfdq2;
				struct sockaddr_in serv_addr, cli_addr;
			    //int n;
			
				char MyHostNamee[_POSIX_HOST_NAME_MAX + 1];
			 	gethostname(MyHostNamee,sizeof MyHostNamee);
			
			
			    sockfdq = socket(AF_INET, SOCK_STREAM, 0);
			    if (sockfdq < 0) 
			    	std::cout<<"ERROR opening socket\n";
			    optval = 1;
			    setsockopt(sockfdq, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
			    bzero((char *) &serv_addr, sizeof(serv_addr));
			    portno = PORT;
			    serv_addr.sin_family = AF_INET;
			    serv_addr.sin_addr.s_addr = INADDR_ANY;
			    serv_addr.sin_port = htons(portno);
			    while (bind(sockfdq, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
			    	continue;
			    listen(sockfdq,MAX_CONN_COUNT);
				std::cout<<"waiting on accept\n\n";
				sockfd = accept(sockfdq, (struct sockaddr *) &cli_addr, &clilen);
				if (sockfd< 0) 
					std::cout<<"ERROR on accept\n";

				tv.tv_sec = TIMEOUT_DUR;
				tv.tv_usec=0; 
				if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof tv)) 
				{ 
					perror("setsockopt");
				}
				//first_time=1;
				acceptingConn=0;
				system("service asterisk restart");
				continue;
			}
			std::cout<<"Message received by the RECEIVE THREAD in the BEGINING(Buffer) is  "<<buffer<<std::endl;
			       
			buffer[n]='\0';
			std::string breakup;
			int index(0);
			while(buffer[index]!='\0')
			{
				breakup += buffer[index];
				index++;  
			}
			
			std::cout<<"the message received is "<< breakup << std::endl;
			//if message received is yes, then I need to become the master
	        if(breakup == "yes" ) {
				std::cout<<"inside yes\n";
		        //I start the asterisk app and then bring up the ip
				//system("/sbin//ifup eth2");
				system("/etc/init.d/asterisk start");
				isMaster=1;
			} else if (breakup == "down" ) {
				std::cout<<"Asterisk problem at the other system. I am becoming the master\n";
				
		        //I start the asterisk app and then bring up the ip
		        	//system("/sbin/ifup eth2");
				system("/etc/init.d/asterisk start");
				isMaster=1;
		        sleep(SLEEP_DUR);
			} else if(breakup == "ok") {
                if(isInitMaster) {
                    std::cout << "";
                }
                system("/etc/init.d/asterisk stop");
            }
		}
		else // IS the master
		{
			n = recv(sockfd,buffer,RECV_BUF_SIZE-1, 0);
			if (n <= 0) {
				if(stop_recv==1)
				{
					//our sys down stop recv
					continue;
				}
				acceptingConn=1;
				std::cout<<"end system not well_2"<<std::endl;
				close(sockfd);
				close(sockfdq);
				//system("service asterisk stop");
				sleep(SLEEP_DUR);
			        //I start the asterisk app and then bring up the ip
				//system("/sbin/ifup eth2");
				system("service asterisk start");
				isMaster=1;
				
				//int sockfdq2;
				struct sockaddr_in serv_addr, cli_addr;
			    int n;
			
				char MyHostNamee[_POSIX_HOST_NAME_MAX + 1];
			 	gethostname(MyHostNamee,sizeof MyHostNamee);
			
			
			    sockfdq = socket(AF_INET, SOCK_STREAM, 0);
			    if (sockfdq < 0) 
			    	std::cout<<"ERROR opening socket\n";
			    
			    optval = 1;
			    setsockopt(sockfdq, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
			    bzero((char *) &serv_addr, sizeof(serv_addr));
			    portno = PORT;
			    serv_addr.sin_family = AF_INET;
			    serv_addr.sin_addr.s_addr = INADDR_ANY;
			    serv_addr.sin_port = htons(portno);
			    while (bind(sockfdq, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
				    continue;
                }
			    listen(sockfdq,MAX_CONN_COUNT);
				sockfd = accept(sockfdq, (struct sockaddr *) &cli_addr, &clilen);
				if (sockfd< 0) { 
					std::cout<<"ERROR on accept\n";
                }

				tv.tv_sec = TIMEOUT_DUR;
				tv.tv_usec=0; 
				if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof tv)) 
				{ 
					perror("setsockopt");
				}
				//first_time=1;
				acceptingConn=0;
				system("service asterisk restart");
				continue;
			}
			std::cout<<"Message received by the RECEIVE THREAD in the BEGINING(Buffer) is  "<<buffer<<std::endl;
			       
			buffer[n]='\0';
			std::string breakup;
			int index(0);
			while(buffer[index]!='\0')
			{
				breakup += buffer[index];
				index++;  
			}
			
			std::cout<<"the message received is "<<breakup << std::endl;

			if(breakup == "down" )
	        	{
				std::cout<<"Asterisk problem at the other system.\n";
			}
		}
	}
	
}



int create_socket() {

     int sfd = -1;
     struct sockaddr_un addr;
                               
     sfd = socket(AF_UNIX, SOCK_STREAM, 0);
     if (sfd == -1) {
         perror("socket");
         exit(-1);
     }

     int reuse = 1;
     if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1) {
         perror("Can NOT set address to reuseable");
         exit(-1);
     }

     memset(&addr, 0, sizeof(struct sockaddr_un));
     /* Clear structure */
     addr.sun_family = AF_UNIX;
     strncpy(addr.sun_path, FAILOVER_SOCKET_PATH, sizeof(addr.sun_path) - 1);

     if (bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
         if(errno == EADDRINUSE) {
             printf("Failover is already in running\n");
         }
         perror("Can not bind on unix socket");
         exit(-1);
     }
    
     std::ofstream pidFile(FAILOVER_PID_PATH);
     pidFile << getpid();
     pidFile.close();

     return sfd;
}

static struct sigaction old_act;
typedef void (*custom_sa_handler)(int);

static void sig_term_handler(int signum) {
    custom_sa_handler old_handler = old_act.sa_handler;

    //if(NULL == old_handler) {
    //    printf("Unexpected exception\n");
    //    exit(-1);
    //}
    
    //(*old_handler)(signum);
    
    unlink(FAILOVER_SOCKET_PATH);
    exit(signum + 128);
}

static void sig_int_handler(int signum) {

    unlink(FAILOVER_SOCKET_PATH);
    exit(signum + 128);
}

int main(int argc, char *argv[])
{
    int test = 0;
	if(1 < argc) {
        if(0 == strcmp(argv[1], "-x")) {
            test = 1;
        }
    }
    
    struct sigaction term_act;
    struct sigaction int_act;

    memset(&term_act, 0, sizeof(term_act));
    term_act.sa_handler = &sig_term_handler;
    
    if(sigaction(SIGTERM, NULL, &old_act) == -1) {
        perror("Can not get old handler of term signal");
        exit(-1);
    }
    
    if(sigaction(SIGTERM, &term_act, NULL) == -1) {
        perror("Can not set new handler of term signal");
        exit(-1);
    }
    
    memset(&int_act, 0, sizeof(int_act));
    int_act.sa_handler = &sig_int_handler;
    if(sigaction(SIGINT, &int_act, NULL) == -1) {
        perror("Can not set int signal");
        exit(-1);
    }

    int sfd = create_socket();
    if(-1 == sfd) {
        perror("Can not create unix socket");
        exit(-1);
    }

    if(test) {
        unlink(FAILOVER_SOCKET_PATH);
        return 0;
    }

    isMaster = 0;
    isInitMaster = isMaster;

	if(!isMaster)
	{
		system("/sbin/ifdown eth2");
		system("/etc/init.d/asterisk stop");
		first_time=0;
		first_time_recv=1;
		std::cout<<"starting the threads"<<std::endl;
		std::thread receive(receiveMessage);
		receive.join();
	}
	else
	{
		//system("/sbin/ifup eth2");
		system("/etc/init.d/asterisk start");
		first_time_recv=0;
		first_time=1;
		initial_exec=1;
		std::cout<<"starting the threads"<<std::endl;
		std::thread send(sendMessage);
		send.join();
	}

    unlink(FAILOVER_SOCKET_PATH);
    close(sfd);
	
	return 0;
}