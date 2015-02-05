#include "project.h"
using namespace cv;
using namespace std;
using namespace Project;
/* For a good starting point on sockets, see : http://www.linuxhowtos.org/C_C++/socket.htm */

/* This thread processes user input to the program   */

void* Project::InputCommandThread(void* arg) 	// thread for processing user input to the program  
{	
	int inputSocket, clientSocket, res;
	sockaddr_in serverAddress, clientAddress;	// serverAddress: address to accept connections, clientAddress: address of client
// 	char messageBuffer[1024];
	cout << "BroadcasterThread starts "<< endl;
 	InitalizeCommandSocket( inputSocket, serverAddress );
	while(1)
	{
		int c = sizeof(struct sockaddr_in);
		clientSocket = accept( inputSocket, (sockaddr*)&clientAddress,  (socklen_t*)&c ) ;
		CheckError( clientSocket, "Accept" );
		stringstream ss;
// 		memset( (void*)messageBuffer, 0, sizeof(messageBuffer) );
		HandleClient( clientSocket );
	}
}

void Project::HandleClient(int clientSocket)
{
	/*
     * 1. Get the Command
     */
		InputCommand cmd;
		int count = 0;
        timespec timeout;
        timeout.tv_sec=15; // wait for 15 sec for input command to arrive
        timeout.tv_nsec=0;
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(clientSocket, &readfds);
		/* wait for incoming data on socket == socket becomes readable */
        int selectRet = pselect(clientSocket+1, &readfds, NULL, NULL, &timeout, NULL);
		if(selectRet == -1) /* if pselect failed  */
		{
			CheckError( selectRet, "HandleClient" );
		}
		else if( selectRet == 0) /* in case of time-out */
		{
			cout << "HandleClient, input timeout" << endl;
			return;
		}
		count = readFromSocket( clientSocket, (char*)&cmd, sizeof(cmd));
		if(count < 0)
        {
			CheckError( count, "readFromSocket" );
		}
		if( count < sizeof(cmd) )
		{
			throw runtime_error("readFromSocket: fewer bytes read, than expected");
		}
	/*
	 * 2. Process the command 
	 */
		switch( cmd.code ) 
		{
			case CmdSetDisplayOff:
				LockMutex( &mutexParameters);
				displayOn = false;
				UnlockMutex( &mutexParameters );
				break;
			case CmdSetDisplayOn:
				LockMutex( &mutexParameters);
				displayOn = true;
				UnlockMutex( &mutexParameters );
				break;
			case CmdSetSaturationLimit:
				LockMutex( &mutexParameters);
				saturationLimit = cmd.data[0]; // first byte
				UnlockMutex( &mutexParameters );
				break;
			case CmdInvalid:
			default:
				throw runtime_error("Invalid command received!");
		}
	/*
	 * 3. Reply ACK code 
	 */
		stringstream answer;
		answer << "ACK " << cmd.code ;
		// wait for socket to become writeable
		timeout.tv_sec=15;
        timeout.tv_nsec=0;
        fd_set writefds;
        FD_ZERO( &writefds );
        FD_SET( clientSocket, &writefds);
        selectRet = pselect(clientSocket+1, NULL, &writefds, NULL, &timeout, NULL);
		if(selectRet==-1)
        {
			CheckError( selectRet, "HandleClient" );
		}
		else if( selectRet == 0) /* in case of time-out */
		{
			cout << "HandleClient, output timeout" << endl;
			return;
		}
		
		count = writeToSocket( clientSocket, answer.str().c_str(), answer.str().length() );
		if(count < 0)
        {
			CheckError( count, "writeToSocket" );
		}
		if( count < answer.str().length() )
		{
			throw runtime_error("writeToSocket: fewer bytes sent than expected");
		}
		return;
}

void Project::InitalizeCommandSocket(int& socketFileDescriptor, sockaddr_in& address)
{
	int rV = 0;
	// create stream socket
	socketFileDescriptor =  socket( AF_INET, SOCK_STREAM, 0);
	CheckError( socketFileDescriptor, "InitalizeCommandSocket:socket" );
	memset( (void*)&address, 0, sizeof(address) );	// clear address
	address.sin_family = AF_INET;	// set address family
	in_addr_t ina = INADDR_ANY;
 	memcpy( &address.sin_addr, &ina, sizeof(ina));
//  	address.sin_addr =(in_addr) 0 ; // accept connection from anywhere   This sets it to INADDR_ANY
//	INADDR_ANY == 0x00000000
	address.sin_port = htons(8888);	// set port number
	// bind socket to address
	rV = bind( socketFileDescriptor, (struct sockaddr *) &address, sizeof(sockaddr_in) );
	CheckError( rV, "InitalizeCommandSocket:bind" );
	listen( socketFileDescriptor, 5 );
	

}
