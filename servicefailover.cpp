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
 

//ports used for communication

#define PORT "44444"


 
// get1 sockaddr, IPv4 or IPv6:
void *get1_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
 
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int portno;
char *name;

void sendRequest( char *reqms)
{
    char MyHostName[_POSIX_HOST_NAME_MAX + 1];
    gethostname(MyHostName,sizeof MyHostName);
    std::string myname(MyHostName);

 
    std::ifstream readsystem("system.in");
    std::string servername;
   
    /* std::string messagetoServer;

    while(readsystem >> servername)
      {
         if( servername != myname )
            {
               messagetoServer = servername;
            }
      }*/
      servername="asterisk3.utdallas.edu";
      
       char *messageTo=new char[servername.length()];  //="asterisk2.utdallas.edu"; //
    servername.copy(messageTo, servername.length());
    messageTo[servername.length()]='\0';
   
   int sockfd, numbytes;  
    struct addrinfo hints1, *servinf, *q;
    int rv;
    char s[INET6_ADDRSTRLEN];
 
    memset(&hints1, 0, sizeof hints1);
    hints1.ai_family = AF_UNSPEC;
    hints1.ai_socktype = SOCK_STREAM;
 
    if ((rv = getaddrinfo(messageTo, PORT, &hints1, &servinf)) != 0) {
       std::cout<<"\nerror getting the addr info"; 
       return;
    }
 
    // loop through all the results and connect to the first we can
    for(q = servinf; q != NULL; q = q->ai_next) {
        if ((sockfd = socket(q->ai_family, q->ai_socktype,
                q->ai_protocol)) == -1) {
               std::cout<<"client: socket";
            continue;
        }
 
        if (connect(sockfd, q->ai_addr, q->ai_addrlen) == -1) {
            close(sockfd);
            std::cout<<"client: connect";
            continue;
        }
 
        break;
    }
 
    if (q == NULL) {
        std::cout<<"stderr,client: failed to connect\n";
        return;
    }
 
    inet_ntop(q->ai_family, get1_in_addr((struct sockaddr *)q->ai_addr), s, sizeof s);
   printf("client: connecting to %s\n", s);
   std::cout<<"this server is connecting to the other server, to pass the exit message:"<< s;
 
    freeaddrinfo(servinf); // all done with this structure
     
     if((numbytes=send(sockfd,reqms,sizeof(reqms)/*MAXDATASIZE-1*/,0)) == -1)
  { std::cout<<"there was an error on forwarding the request to the other server"<<messageTo<<std::endl;
     return;  }
 
bzero(reqms,0);
 
   close(sockfd);
 

    return;
}



//we will have 2 threads working always on the system
/* the first one will be to send the messages to the other server and the other thread would be to receive the messages from the other server
*/




void sendMessage()
{

 static int errorStatus;
ad:   

 while (1)
    {
sleep(10);
    //if I have eth2:1 on my system, then I have asterisk app on myself

    system("/sbin/ifconfig -a | grep eth2:1 | wc -l  > check.log");
    // system("ethtool eth2 | grep Link detected: no | wc -l");

    std::ifstream check("check.log");
    int getcount(0);
    check>>getcount;
        check.close();
        //remove("check.log");
    std::cout<<getcount;

        if(getcount==1)//I am no more the master
        {
        std::cout<<"entering 1 " <<std::endl;
        std::string whatTosend="no";
        char *Mess= new char[whatTosend.length( )];
        whatTosend.copy(Mess,whatTosend.length( ));
        Mess[whatTosend.length()]='\0';

        // send the message no action from my side
            std::thread message1(sendRequest, std::ref(Mess));
                    message1.join();
           sleep(10);
                   goto ad;      
        }

        if(getcount==0)
        {
        //I need to check my system state
        std::cout<<"entering 2" <<std::endl;

            //condition 1.. (if I get errors in my /var/log/message repeatedly, I need to migrate the service)
        system("tail -500 /var/log/messages | grep error | wc -l > error.log");
                  std::ifstream checkerror("error.log");
                  int howmanyErrors;
                  checkerror>>howmanyErrors;
                  checkerror.close();
                 // remove("error.log");
                  if(howmanyErrors >=100)
                  {
                    errorStatus++;
                  }
                  //if I get these many errors repeatdely say for twice, i.e. error frequency is high, we migrate the service
                  if(errorStatus ==2)
                   {
std::cout<<"entering 3" <<std::endl;

                     system("/sbin/ifdown eth2"); 
                     system("/sbin/ifdown eth2:1");
                      sleep(5);// we bring down the interface and the asterisk service on the system
                     system("/etc/init.d/asterisk stop");
                      sleep(5);
                     std::string whatTosend="yes";
             char *Mess= new char[whatTosend.length( )];
             whatTosend.copy(Mess,whatTosend.length( ));
             Mess[whatTosend.length()]='\0';

            // send the message no action from my side
            std::thread message2(sendRequest, std::ref(Mess));
                    message2.join();
                  sleep(3);
               system("/sbin/ifup eth2"); 
                  sleep(3);
                 goto ad;
      
                  }
              //if the process goes down on the system
                  system("ps -ef | grep asterisk | wc -l > processcount.log ");
                 
                  std::ifstream procount("processcount.log");
                  int counter;
                  procount>>counter;
                  procount.close();
              // remove("processcount.log");
                  if( counter == 1 || counter == 2 ) //application is down or not fully functional
                   {
std::cout<<"entering 4 " <<std::endl;

                     system("/sbin/ifdown eth2"); // we bring down the interface and the asterisk service on the system
                     sleep(2);
                     system("/etc/init.d/asterisk stop");
                     sleep(2);
                     std::string whatTosend="yes";
             char *Mess= new char[whatTosend.length( )];
             whatTosend.copy(Mess,whatTosend.length( ));
             Mess[whatTosend.length()]='\0';


            // send the message no action from my side
            std::thread message3(sendRequest, std::ref(Mess));
                    message3.join();
                 sleep(3);
               system("/sbin/ifup eth2"); 
                  sleep(3);
                   goto ad;
                   }

                  //if the defunct process count goes high
                  system("ps -ef | grep defunct | wc -l > defunct.log ");
                  std::ifstream defcount("defunct.log"); 
                  int zombiecount;
                  defcount >> zombiecount;
                  defcount.close();
             //  remove("defunct.log");
                  if(zombiecount >100)
                  {
std::cout<<"entering 5 " <<std::endl;

                    system("/sbin/ifdown eth2"); // we bring down the interface and the asterisk service on the system
                    sleep(2);
                     system("/etc/init.d/asterisk stop");
                     sleep(2);
                     std::string whatTosend="yes";
             char *Mess= new char[whatTosend.length( )];
             whatTosend.copy(Mess,whatTosend.length( ));
             Mess[whatTosend.length()]='\0';

            // send the message no action from my side
            std::thread message3(sendRequest, std::ref(Mess));
                    message3.join();
                 sleep(3);
               system("/sbin/ifup eth2"); 
                  sleep(3);
                  goto ad;
                  
                  } 


        }   



    }
}



void receiveMessage()
{
//let us setup the receving socket
     int sockfdq, newsockfdq, portno;
     socklen_t clilen;
     
     struct sockaddr_in serv_addr, cli_addr;
     int n;

 char MyHostNamee[_POSIX_HOST_NAME_MAX + 1];
 gethostname(MyHostNamee,sizeof MyHostNamee);


     sockfdq = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfdq < 0) 
        std::cout<<"ERROR opening socket";
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = 44444;//atoi(argv[2]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfdq, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
              std::cout<<"ERROR on binding";
     listen(sockfdq,50);
     
 
//always listening on the port to read the messages
       while(1)
    {   
            char *buffer=new char[256];
        clilen = sizeof(cli_addr);
             
             newsockfdq = accept(sockfdq, (struct sockaddr *) &cli_addr, &clilen);
                if (newsockfdq< 0) 
                   std::cout<<"ERROR on accept";
 
                   bzero(buffer,256);
                   sleep(0.8);
                             
                n = recv(newsockfdq,buffer,255, 0);//read(newsockfdq,buffer,strlen(buffer));
                 if (n < 0) std::cout<<"ERROR reading from socket";
                  std::cout<<"Message received by the RECEIVE THREAD in the BEGINING(Buffer) is  "<<buffer<<std::endl;
       
             
   
           buffer[n]='\0';
             std::string breakup;
             int index(0);
             while(buffer[index]!='\0')
                {
                breakup += buffer[index];
                index++;  
                }
std::cout<<"the message received is "<<breakup;
//if message received is yes, then I need to become the master
         if(breakup == "yes")
            {
std::cout<<"received 0 " <<std::endl;

              //I start the asterisk app and then bring up the ip
              system("/etc/init.d/asterisk start");
              sleep(5);
               //if I have the ip by mistake then I need to bring it down first
              system("/sbin/ifconfig -a | grep eth2:1 | wc -l > interface.log");
              std::ifstream interface("interface.log");
              int logicalinterface;
              interface>>logicalinterface;
              interface.close();
              remove("interface.log");
              if(logicalinterface == 0 ) { ; } //things are good
              else { system("/sbin/ifdown eth2");
                     sleep(1);
                   }
              system("/bin/sh /float/floatscript.sh");
            }


         }

}

int main(int argc, char *argv[])
{


//name = argv[1];
//portno = atoi(argv[1]);

std::cout<<"starting the receive thread"<<std::endl;
std::thread receive(receiveMessage);

sleep(10);
std::cout<<"starting the send thread"<<std::endl;
std::thread send(sendMessage);

send.join();
receive.join();
return 0;
}
