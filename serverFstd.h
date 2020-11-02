/*Michael Kulinich
 * kulinich@chapman.edu
 * serverFstd.h
 * A server in the internet domain using TCP
*/
#ifndef SERVER_H
#define SERVER_H

#include "Fan.h"
#include <thread>
#include <mutex>
#include <vector>

#define PORT 8080
//#define NFANS    4
//#define NSENSORS 3
// set it to large N or change "for" loop -> while(1) ...
#define MAX_NUMBER_CYCLES 1000     
#define LOW_TEMP   25. 
#define HIGH_TEMP  75.
#define HIGH_DUTYCYCLE 1.
#define LOW_DUTYCYCLE  .2
#define MIN_TEMP  -237.15

#define T_PERIOD   1.5  //  1.0 sec
#define LOOP_WAIT  200 // 200 msec
#define MAX_NUMBER_WAITS (int(T_PERIOD/(LOOP_WAIT*1.e-3))+1)
//struct that has all the data necessary for the socket threads
struct ThreadStructuredData {
  int thread_id;
  int socket_file_descriptor;

  // SHARED MEMORY - POINTERS
  // pointer to individual Counter MEM location
  int*       counter_memory;
  // pointer to Temperature MEM location
  float* temperature_memory; 
};
//struct that has all the data necessary for master thread
struct MasterStructuredData {
  int thread_id;
  uint number_of_fans;
  uint number_of_sockets;
  
  // FAN DATA 
  std::vector<Fan> fans;
  
  // SHARED MEMORY - POINTERS
  // pointer to individual Counter MEM location
  int*       counter_memory;    //beginning_counter_memory
  // pointer to Temperature MEM location
  float* temperature_memory; 
};


class Server{

 public:
  Server();
  //given the file name, it calls readConfig 
  //to read the configuration file and pars
  //the number of fans and sockets
  Server(const char* );
  ~Server();
  
  //called from main, runs the server and creates all the necessary threads,
  //data, and mutexs. calls three functions:
  //void createMasterThread(), int createSocketThreads(), void tearDown(int sockfd);
  void Run();

 private:
  uint m_number_of_sockets;
  uint m_number_of_fans;

  // FUNCTIONS CALLED IN RUN
  void createMasterThread();
  int createSocketThreads();
  //closes the socket and joins with master thread
  void tearDown(int sockfd);
  
 
  // THREAD FUNCTIONS
  void socketFunction( ThreadStructuredData* );
  void masterFunction( MasterStructuredData* );
  
  // FAN FUNCTIONS
  void setDutyCycle(float temp);
  float getDutyCycle();
  
  static void error( const char *msg );
  //read the configuration file, called in constructor
  bool readConfig(const char*, uint&, uint&);
  
  
  // FAN DATA
  std::vector<Fan> m_fans;
  float m_duty_cycle;
  
  // THREAD DATA
  std::vector<ThreadStructuredData> m_socket_thread_data; 
  MasterStructuredData              m_master_thread_data;
  
  // SHARED DATA
  int   * m_counter_memory; 
  float * m_temperature_memory; 

  // THREADS
  std::vector<std::thread*>  m_socket_thread; 
  std::thread*               m_master_thread;

  // MUTEXES
  std::vector<std::mutex*> m_socket_mutex; 
  std::mutex*              m_master_mutex;            

};


#endif // SERVER_H
