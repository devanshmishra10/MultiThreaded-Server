# MultiThreaded-Server
## Project Description
A client-server model which processes the request one by one does not seem good to the user as the time delay for each request to be processed will be large.
So what can one do… ?
Creating a Multithread Server can be an answer to this problem.
This project is a use case application for multithreaded server in which I've developed CHATPLACE , which is like a common chatroom on terminal where different clients can join servers running at different ports.
## How to run the project
* you can choose Download Zip option in the dropdown of 'code' just above the files or can use git clone command to download the contents of the project.
```
$ git clone https://github.com/devanshmishra10/MultiThreaded-Server
```
* Once you have the folder and the files in your system go to your folder directory and compile both the files in following way.
```
$ gcc server.c -o server -pthread
```
```
$ gcc client.c -o client -pthread
```
> Note : Make sure to run this project in Linux distribution (Ubuntu) or WSL in order to not get any error.
* After compiling the files open different terminal windows and start a server by:
```
$ ./server <port_number>    
```
* Now in another terminal connect a client with a server by:
```
$ ./client <port_number>
```
> Make sure to enter same port number for a client and server connection, E.g.
```
$ ./server 9090
```
```
$ ./client 9090
```
* You can make different clients join at any server and even start servers at different ports.
* Whenever you make a client join any server you will be propmted to enter a name , so enter the name and start messaging.It would be displayed in the corresponding server and to the different clients in the same server.

## Description of my tasks
* This project is basically a type of a common chatroom of multiple clients joined in any server where each client can send a message which would be visible in that server and to each other client in the same server. The clients at any server running at another port would not be able to see those messages.
* Whenever a new client joins or someone already in the server leaves a message would be displayed in the corresponding server and to other clients on that server to make them aware of that.
## The internal working of the project
* Multithreading is the concept of using multiple threads to execute concurrently. 
* In order to make the server more responsive the requests need to be assigned to different threads, so that the maximum number of requests can be processed concurrently.It allows us to create multiple threads for concurrent process flow. 
* In a Unix/Linux operating system, the C/C++ languages provide the POSIX thread(pthread) standard API(Application program Interface) for all thread related functions. In this project I've used pthread library.
* Using many threads would lead to a situation where two threads are accessing the same code or the same shared variable and there is a possibility of wrong output. To tackle this situation mutex locks have been used to allow only one thread to access the shared code and do the necessary changes and synchronize all the threads. 
* Different functions in code are responsible for handling different processes.
> The main function in both files takes the input of server and client port respectively and display welcome message if the connection is successful.
> 
> The functioning of enqueue and dequeue operations on the clients when adding or removing from a server is handled in server file.
> 
> message_sent() and message_recvd() functions handle the flow of messages being written by clients which need to be updated in server and different clients in the same server.
> 
> handle_connec() function in server.c takes care if any client has entered or left the chat running at that server.  
## My learnings from the project
* Through this project I got to learn about how does socket programming work. 
* Learnt how to establish a client server connection and use multithreading and mutex lock alongside it for better results. 
## Resources
* [Thread Functions in C/C++](https://www.geeksforgeeks.org/thread-functions-in-c-c/)
* [MultiThreading in C](https://www.geeksforgeeks.org/multithreading-c-2/?ref=lbp)
* [Socket Programming in C/C++](https://www.geeksforgeeks.org/socket-programming-cc/)


https://user-images.githubusercontent.com/86547969/174862558-ee00643d-eb77-4791-97fd-e4da4e886b94.mp4
