/*Michael Kulinich
 * kulinich@chapman.edu
 * Intuitive Challenge c++
 * serverFstd.cpp
 * A server class
*/

#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include "serverFstd.h"
using namespace std;



Server::Server()
{
  // TODO
  m_number_of_sockets=3;
  m_number_of_fans=4;
  m_duty_cycle = 1.;
}

Server::Server(const char* FileNm)
{
  m_duty_cycle = 1.;
  uint N1,N2;
  if(readConfig(FileNm,N1,N2))
  {
    m_number_of_sockets=N1;
    m_number_of_fans=N2;
  }
  
  // FAN DATA
  m_fans = std::vector<Fan>(m_number_of_fans);
  
  // THREAD DATA
  m_socket_thread_data = std::vector<ThreadStructuredData>(m_number_of_sockets);
  
  // SHARED DATA
  m_counter_memory     = new int[m_number_of_sockets];
  m_temperature_memory = new float[m_number_of_sockets];
  
  // THREADS
  m_socket_thread = std::vector<std::thread*>(m_number_of_sockets);
  m_socket_mutex  = std::vector<std::mutex*>(m_number_of_sockets);
  
  for(auto & m : m_socket_mutex )
  {
    m = new std::mutex();
  }
  
  m_master_mutex = new std::mutex();
  
  cerr << " m_number_of_sockets=" << m_number_of_sockets << " m_number_of_fans=" <<  m_number_of_fans << "\n" << endl ;
}


Server::~Server()
{
  // TODO
  //delete [] m_socket_thread_data;
  delete [] m_counter_memory;
  delete [] m_temperature_memory;
  for( auto& t : m_socket_thread ){ delete t; }
  for( auto& m : m_socket_mutex  ){ delete m; }
  delete m_master_thread;
  delete m_master_mutex;
}

void Server::Run()
{
  createMasterThread();
  //Initialize sockfd to the file descriptor of the socket created
  int sockfd = createSocketThreads(); 
  tearDown(sockfd);
}


void Server::createMasterThread()
{
    
  //initialize the fans and their Max PWM count
  for( uint fan_index = 0; fan_index < m_number_of_fans; fan_index++ )
  {
    uint diff_max = 1234+fan_index*111; // different Max 
    m_fans[fan_index]  = Fan(diff_max);  
  }
  
  m_master_thread_data.thread_id              = -1;
  m_master_thread_data.number_of_fans         = m_number_of_fans;  
  m_master_thread_data.number_of_sockets      = m_number_of_sockets;  
  m_master_thread_data.fans                   = m_fans;

  
  m_master_thread_data.counter_memory         = &m_counter_memory[0];
  

  m_master_thread_data.temperature_memory     = &m_temperature_memory[0];

  
  // create master thread  
  m_master_thread =
    new std::thread( [=] { Server::masterFunction( &m_master_thread_data ); } );
}


int Server::createSocketThreads()
{
  // --------------- NETWORKING AND SOCKETS ---------------//
  int sockfd, newsockfd, portno, pid;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;

  // create a socket and get its file descriptor
  sockfd = socket( AF_INET, SOCK_STREAM, 0 );
  if(sockfd < 0) 
    error("ERROR opening socket");

  // clear buffer and set some networking information
  bzero( ( char* ) &serv_addr, sizeof( serv_addr ) );
  serv_addr.sin_family      = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port        = htons( PORT );
  
  int reuse = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
  error("setsockopt(SO_REUSEADDR) failed");
  
  
  // ----------HANDLES ERROR IF PROJECT RAN FEW TIMES IN SEQUENCE---------//
#ifdef SO_REUSEPORT
  if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0) 
  error("setsockopt(SO_REUSEPORT) failed");
#endif
  // bind and check at the same time
  if( bind ( sockfd,
	      ( struct sockaddr* ) &serv_addr,
	      sizeof( serv_addr ) ) < 0 )
  {
      error( "ERROR on binding" );
  }

    
  // listen on this socket
  listen( sockfd, m_number_of_sockets );
  clilen = sizeof( cli_addr );
  
  // -------------- SOCKET THREADS AND MUTEXS -------------//
  for(uint socket_index = 0; socket_index < m_number_of_sockets; socket_index++)
  {
    // default values
    m_counter_memory      [socket_index]  = 0;
    m_temperature_memory  [socket_index]  = HIGH_TEMP + socket_index * 0.5; //start each fan with arbitrary high temp
    
    // get the new socket file descriptor once a client tries to connect
    // so the program sits here every time it is waiting for a new client
    newsockfd = accept( sockfd, ( struct sockaddr* )&cli_addr , &clilen );
    if (newsockfd < 0) 
      error("ERROR on accept");
    
    (m_socket_thread_data[socket_index]).thread_id              = socket_index;
    (m_socket_thread_data[socket_index]).socket_file_descriptor = newsockfd;
    (m_socket_thread_data[socket_index]).counter_memory         = &m_counter_memory[socket_index];
    (m_socket_thread_data[socket_index]).temperature_memory     = &m_temperature_memory[socket_index];

    // create a thread (and connect it with the socket function) and perform a check
    m_socket_thread[ socket_index ] =
      new std::thread( [=] { Server::socketFunction( &m_socket_thread_data[ socket_index ] ); } );
  }
  return sockfd;
}


void Server::tearDown(int sockfd)
{
    close(sockfd);

    for(int socket_index = 0; socket_index < m_number_of_sockets; socket_index++)
    { 
      m_socket_thread[ socket_index ]->join();
    }
    m_master_thread->join();
  
    printf(" After Thread Join \n");
}



void Server::socketFunction(ThreadStructuredData* my_data)
{
  // buffer for data
  char buffer[256];

  // return value of read and write to socket
  int ret = 0;
  
  for(int cycle_number = 0; cycle_number < MAX_NUMBER_CYCLES; cycle_number++)
  {  
    //clear buffer
    bzero( buffer, 256 );
    // read data from socket
    ret = read( my_data->socket_file_descriptor, buffer, 255);
    if ( ret < 0 )
    {
      Server::error("ERROR reading from socket");
    }

    string S1(buffer);
    stringstream ss(S1);
    string tmp1;
    float temperature_reading;
    ss >> tmp1 >> tmp1 >> tmp1 >> temperature_reading ;

    // lock socket mutex 
    m_socket_mutex[ my_data->thread_id ]->lock();
    *(my_data->counter_memory) +=1;
    *(my_data->temperature_memory)  = temperature_reading;   
    m_socket_mutex[ my_data->thread_id ]->unlock();

  } // end for LOOP


}


  
  
/////////////////
//MASTER FUNCTION
/////////////////
void Server::masterFunction(MasterStructuredData* my_data)
{    
  uint Num_fans = my_data->number_of_fans ;         // Number of Fans
  uint num_sockets = my_data->number_of_sockets ;   // Number of Sockets


  std::vector<Fan> fanVect = my_data->fans;
  for(uint fan_index = 0; fan_index < Num_fans; fan_index++)
  {
    m_master_mutex->lock();
    printf(" #fan %d FANmax=%d \n", fan_index+1,fanVect[fan_index].getMaxPWMCount()); 
    m_master_mutex->unlock();
  }

  // to finish before MAX_NUMBER_CYCLES  diff_clients will start sending at diff time 
  for(int cycle_number = 0; cycle_number < MAX_NUMBER_CYCLES ; cycle_number++)
  {   
    m_master_mutex->lock();    
    printf("BEGIN CYCLE %d \n",cycle_number);
    m_master_mutex->unlock();
    int* number_of_messages_in_socket = new int[num_sockets];
    for(uint i=0;i<num_sockets;i++) { number_of_messages_in_socket[i] = 0;}
    int m_number_of_sockets_with_message = 0;
    int number_of_waits = 0;
    
    //wait for all sockets to send in message, if not received message 
    //from 1+ sockets in a certain amount of time, it will fill those sockets 
    //with a HIGH_TEMP
    while(m_number_of_sockets_with_message < num_sockets) //m_number_of_sockets_with_message is incremented at end of while loop
    {
      usleep(LOOP_WAIT*1e3); // micro_sec
      for(uint socket_index = 0; socket_index < num_sockets; socket_index++) 
      {
        if(number_of_messages_in_socket[ socket_index ]) continue;
        int counter_value = *( my_data->counter_memory + socket_index );
        if( counter_value ) 
        {
          number_of_messages_in_socket[ socket_index ] = counter_value;
          m_number_of_sockets_with_message++; //increments so we can exit out of while loop eventually
        }
      } // end for loop
      
      number_of_waits++;

      
      //if one of the sockets hasn't sent its temperature for a cycle, turn the max temp to HIGH_TEMP
      //this is a percaution so that nothing will burn in the robot if a sensor broke!
      if(number_of_waits > MAX_NUMBER_WAITS)
      {
        for(int socket_index = 0; socket_index < num_sockets; socket_index++) 
        {
        
          // if its 0 in any one of the sockets, one of the senors maybe broke so turn max temp to HIGH_TEMP
          if(!number_of_messages_in_socket[ socket_index ]) 
          {          

	      // lock socket mutex
	        m_socket_mutex[ socket_index ]->lock();
	        *( my_data->temperature_memory + socket_index ) = HIGH_TEMP;// + socket_index * .5; //
	        m_socket_mutex[ socket_index ]->unlock();
	    
	        // increment number of total sockets that have at least one message
	        m_number_of_sockets_with_message++;
	      } 
	    }

	    // stop waiting for messages on the sockets for this cycle
	    // well just continue as if normal and we have our temps (except there can be HIGH_TEMP from one or more sockets)
	    break;
      }
    } // end while loop
    
    delete []number_of_messages_in_socket;
    
    // reset the socket counter memory
    for(uint socket_index = 0; socket_index <num_sockets; ++socket_index)
    {
      // lock socket mutex
      m_socket_mutex[ socket_index ]->lock();
      *( my_data->counter_memory + socket_index ) = 0;
      m_socket_mutex[ socket_index ]->unlock();
    }

    float Max_Temperature = -237.15 ; // Celsius
    if( m_number_of_sockets_with_message >= num_sockets )
    {
        
	  m_master_mutex->lock();
	  for( int socket_index = 0; socket_index < num_sockets; ++socket_index)
      {
	    float current_socket_temp=*( my_data->temperature_memory + socket_index );
	    if(current_socket_temp>Max_Temperature) Max_Temperature=current_socket_temp;
	    printf("\tSocket# %d: Temperature = %7.2f C\n",socket_index+1,current_socket_temp );
	  }
      printf("\n##########\nMax Temperature =%7.2f C\n", Max_Temperature);
      
      //set the duty cycle based on the Max_Temperature
      setDutyCycle( Max_Temperature );
      float duty_cycle = getDutyCycle();
      printf("Duty Cycle      =%7.2f %\n##########\n", duty_cycle * 100);
      
      for(int fan_index = 0; fan_index < Num_fans; ++fan_index)
      {
        fanVect[ fan_index ].setPWMCount(m_duty_cycle);
        printf("\tFan# %d: PWM Count = %d / %d \n", fan_index+1, fanVect[ fan_index ].getCurrentPWMCount(), fanVect[ fan_index ].getMaxPWMCount());
      }
      
      printf("\nEND OF CYCLE %d \n\n", cycle_number);
	  m_master_mutex->unlock();
    }
  } // end for loop
}

void Server::setDutyCycle(float temp)
{
  float slope = (HIGH_DUTYCYCLE-LOW_DUTYCYCLE)/(HIGH_TEMP-LOW_TEMP);
  float duty_cycle = slope*temp - (slope * LOW_TEMP) + LOW_DUTYCYCLE;
  if(duty_cycle > 1.) m_duty_cycle = 1.;
  else if(duty_cycle < 0) m_duty_cycle = 0.;
  else
    m_duty_cycle = duty_cycle;//slope*temp - (slope * LOW_TEMP) + LOW_DUTYCYCLE;
}

float Server::getDutyCycle()
{
  return m_duty_cycle;
}

void Server::error(const char *msg)
{
    perror(msg);
    exit(1);
}

bool Server::readConfig(const char* ConfigFileName, uint& Nsockets, uint& Nfans )
{
  ifstream in ;
  string line, var_name;
  string s1("NSOCKETS");
  string s2("NFANS");
  
  int Nn ;//Nsockets or Nfans;
  bool ret_status=false;
  in.open(ConfigFileName);
  if(in.fail()) {
    std::cout << " Fail to open Config file "<< std::endl ;
    return ret_status;
  }
  stringstream ss;
  
  for(uint i=0;i<2;i++) {
    getline (in,line);
    ss.str(line);
    ss >> var_name >> Nn ;
    if(ss.fail()) {                   // if an error happened
      cout << " ERR reading " << var_name << endl ;
      return ret_status;
    }
    ss.clear();
    ss.str(std::string());    
    if(var_name.compare(s1)==0){
      Nsockets=Nn;
      continue;
    }


    if(var_name.compare(s2)==0){
      Nfans=Nn;
      continue;
    }
    
    std::cout << " name mismatch for ""NSOCKETS"" "<< std::endl ;
    in.close();
    return ret_status;
  } 
  in.close();


  return true;
}

int main(int argc, char *argv[])
{
  //uint Nsockets, Nfans;
  
  Server server("config.txt");
  //  Server server;
  server.Run();
  
  return 0; 
}
