/*Michael Kulinich
 * kulinich@chapman.edu
 * Intuitive Challenge c++
 * client.c
 * A client socket program representing the subsystems
 */
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <time.h>
#include <stdlib.h>
#include <cmath>

#define DEBUG       //Useful when using run.sh and print is going to same terminal as server
#define PORT 8080
#define NUMREADINGS 1000
#define SERV_IP "127.0.0.1"

//creates a random temp from different places
//along the sin wave, based on system time
//return the random float temp
float createTemp()
{
  srand(time(NULL)+(int)getpid());
  return 25*sin(rand()) + 50;
}
int main(int argc, char const *argv[]) 
{ 
    int pid= (int)getpid();
    int sock = 0;        
    struct sockaddr_in serv_addr;
    char HELLO[128];

    //checks if the socket was not created properly
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    serv_addr.sin_family = AF_INET; // TCP/IP
    serv_addr.sin_port = htons(PORT); // host 2-> network Short
       
   //checks if address isn't valid
    if(inet_pton(AF_INET,SERV_IP, &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
   
   // connect for CLIENT'
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    }
    float TempC= createTemp();

    for(int i=0;i<NUMREADINGS; i++) 
    {
#ifdef DEBUG
      sprintf(HELLO,"\33[32m client PID:%d #%d %f \33[0m\n",pid,i,TempC);
      printf("%s", HELLO);
#endif 
      sprintf(HELLO,"client PID:%d #%d %f  \n",pid,i,TempC);  
      send(sock , HELLO , strlen(HELLO) , 0 );
      usleep((pid%4+2)*2e5);  //arbitrary sleeptime for each different client
      TempC = createTemp();
    }    
    return 0; 
} 
