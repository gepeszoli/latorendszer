#include "project.h"

using namespace cv;
using namespace std;
using namespace Project;

void Init()
{
  signal(SIGPIPE, SIG_IGN); // SIGPIPE must be ignored (that is sent when a client connects to the InputCommandThread and terminates
}
int main(int argc, char** argv)
{
	
	Init();
	cout << "Checking for calibration data (out_camera_data.xml)" << endl;
	ifstream calibrationFile ("out_camera_data.xml");
	if( !calibrationFile.is_open() )
	{
		cout << "Calibration needed." << endl; 
	}

	/* Launch the worker threads */
	
	pthread_t threads[5]; // this array holds the pthread_t objects identifying the threads
	
	/* initialize thread attributes for the created threads */
    pthread_attr_t threadAttributes, threadAttributes2, threadAttributes3, threadAttributes4;    // thread attributes
	int rV = pthread_attr_init( &threadAttributes ); // initialize the thread attribute object
    CheckPthreadError( rV, "main:pthread_attr_init");
	rV = pthread_attr_setdetachstate( &threadAttributes, PTHREAD_CREATE_DETACHED );  // make the thread detached (not let other threads wait for it to finish)
	CheckPthreadError( rV, "main:pthread_attr_setdetachstate");
	
	rV = pthread_attr_init( &threadAttributes2 ); // initialize the thread attribute object
    CheckPthreadError( rV, "main:pthread_attr_init");
	rV = pthread_attr_setdetachstate( &threadAttributes2, PTHREAD_CREATE_DETACHED );  // make the thread detached (not let other threads wait for it to finish)
	CheckPthreadError( rV, "main:pthread_attr_setdetachstate");
	
	rV = pthread_attr_init( &threadAttributes3 ); // initialize the thread attribute object
	CheckPthreadError( rV, "main:pthread_attr_init");
	rV = pthread_attr_setdetachstate( &threadAttributes3, PTHREAD_CREATE_DETACHED );  // make the thread detached (not let other threads wait for it to finish)
	CheckPthreadError( rV, "main:pthread_attr_setdetachstate");
	
	rV = pthread_attr_init( &threadAttributes4 ); // initialize the thread attribute object
	CheckPthreadError( rV, "main:pthread_attr_init");
	rV = pthread_attr_setdetachstate( &threadAttributes4, PTHREAD_CREATE_DETACHED );  // make the thread detached (not let other threads wait for it to finish)
	CheckPthreadError( rV, "main:pthread_attr_setdetachstate");
	
	/* Start the threads */
	rV = pthread_create( &threads[ 0 ], &threadAttributes, LocatorThread, NULL );
	CheckPthreadError( rV, "Starting LocatorThread:");
	
	rV = pthread_create( &threads[ 1 ], &threadAttributes2, DisplayThread, NULL );
	CheckPthreadError( rV, "Starting DisplayThread:");
 	
	rV = pthread_create( &threads[ 2 ], &threadAttributes3, BroadcasterThread, NULL );
	CheckPthreadError( rV, "Starting BroadcasterThread:");

	rV = pthread_create( &threads[ 3 ], &threadAttributes4, InputCommandThread, NULL );
	CheckPthreadError( rV, "Starting InputCommandThread:");
	
	/* free up attribute resources */
	rV = pthread_attr_destroy( &threadAttributes );
	CheckPthreadError( rV, "Main:pthread_attr_destroy");
	rV = pthread_attr_destroy( &threadAttributes2 );
	CheckPthreadError( rV, "Main:pthread_attr_destroy");
	rV = pthread_attr_destroy( &threadAttributes3 );
	CheckPthreadError( rV, "Main:pthread_attr_destroy");
	rV = pthread_attr_destroy( &threadAttributes4 );
	CheckPthreadError( rV, "Main:pthread_attr_destroy");
	while(1){ sleep(10);}
	
}


















