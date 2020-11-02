# Fan Control
This project controls fan speeds for a robot with subsystems. Each subsystem is represented as a socket. The goal is to minimize noise level without endangering the electronics. The project has a server and then several clients (depending on how many subsystems are need in configuration). 
The project starts up with the server launching first, it waits for messages from clients. Each client is added one after the other with some wait time. The server waits for a certain time to receive message from client. Before the clients start sending temperatures to server, the duty cycle is set to 100%, to demonstrate a percaution that if a sensor is broken, we should turn fan speeds all the way up. Next, temperatures are generated randomly, using a random position on a sine wave, repeatedly from each socket and the server waits a certain amount of time for at least one message from each socket. Each socket thread sends messages at different time intervals to demonstrate a real world example that is not perfect. Then the most recent temperature measurement is collected from each socket(subsystem) and printed onto the terminal. 
The max temperature recording per cycle is calculated from each socket and then used to set the duty cycle for each fan. As a result of the updated duty cycle, the fans print their current PWM count. The program runs for 10 seconds after each socket starts up, then killed from 'run.sh' for demo reason. 

## Geting Started
These instructions will get you a the project up and running on your local machine. See deployment for notes on how to deploy the project on your system. The configuration of the system is in "config.txt" and the number of fans and sockets can be changed.

### Requirements
Nothing needs to be installed. Linux Fedora 64-bit was used

**Linux**
- g++ compiler


### Compile and Deployment
In order to compile the projects, it is required to have g++.

Make sure the shell script 'run.sh' is an executable, if not write:

```
chmod u+x run.sh
```

.To compile and run, execute the shell script

```
./run.sh
```

.The script will launch Clients in background from the same terminal as Server. If executed from run.sh the client messages are in green to make it easier to see.


Clients can also be launched from "NSOCKETS" terminals (number of sockets you wish to have) on one computer or at different computers (The server IP should be defined by # SERV_IP " 192.zzz.yyy.xxx" in client.c) You need to kill the server manually, or change the number of cycles to a smaller number.

```
g++ serverFstd.cpp Fan.cpp -o server -lpthread

g++ client.c -o client

./server                            [in one terminal]

./client                            [in other terminals, if NSOCKETS = 3, open 3 new terminals to execute]
```



