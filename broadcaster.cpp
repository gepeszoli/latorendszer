#include "project.h"
using namespace cv;
using namespace std;
using namespace Project;

/* This thread broadcasts the located marker positions */

void* Project::BroadcasterThread(void* arg)
{	
	int broadcastSocket, res;
	sockaddr_in address;	// target address
	char messageBuffer[1024];
	cout << "BroadcasterThread starts "<< endl;
 	InitalizeBroadcastSocket( broadcastSocket, address );
	while(1)
	{
		stringstream ss;			// write the message to this stringstream
		LockMutex( &mutexMarkers );	// lock the mutex protecting, the detection data
		if( Markers.size() > 0 ) 
		{
		  for( size_t i = 0; i<Markers.size(); i++)
		  {
		    ss << "{ \"object\" : \"marker\" , \"x\" :\" " << Markers[i].x << "\" , \"y\" : \"" << Markers[i].y <<"\" }" ;   
		  }
		}
		else
		{
		    ss << "{ \"object\":\"0\"}";
		}
		cout << "Broadcast: " << ss.str() << endl;
		res = sendto(broadcastSocket, ss.str().c_str(), ss.str().length(), 0, (struct sockaddr *)&address, sizeof(address) );
		CheckError( res, "sendto" );
		UnlockMutex( &mutexMarkers );
		/* provides processing time for the locator thread */
		/* necessary so that the windows remain visible */
 		usleep(1000);
	}
}

void Project::InitalizeBroadcastSocket(int& socketFileDescriptor, sockaddr_in& address)
{
	socketFileDescriptor =  socket( AF_INET, SOCK_DGRAM, 0);
	CheckError( socketFileDescriptor, "socket" );
	int status, yes;
	status = setsockopt( socketFileDescriptor, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(int) );
	CheckError( status, "setsockopt" );
	
	memset( (void*)&address, 0, sizeof(address) );	// clear address
	address.sin_family = AF_INET;	// set address family
	address.sin_port = htons(8080);	// set port number
	inet_aton("255.255.255.255", &address.sin_addr );
	// create socket
	
	// bind socket to broadcast address
}
