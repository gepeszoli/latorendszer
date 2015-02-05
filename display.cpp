#include "project.h"
using namespace cv;
using namespace std;
using namespace Project;

void* Project::DisplayThread(void* arg)
{	
	cout << "DisplayThread starts "<< endl;
	/* Create windows to show the results of the processing steps */
	namedWindow("edges",CV_WINDOW_NORMAL);		// window for edge detected frame
	namedWindow("segmented",CV_WINDOW_NORMAL);	// window for the result of color segmentation
	namedWindow("original",CV_WINDOW_NORMAL);	// window for the original image + detected markers and circles
	namedWindow("blurred",CV_WINDOW_NORMAL);	// window for the blurred image (which reduces false edge detection)
	
	while(1)
	{
		LockMutex( &mutexParameters );	// lock the mutex protecting the image data
		if( displayOn && displayInitialDataAvailable ) // only display something if it is enabled
		{
			UnlockMutex( &mutexParameters );
			LockMutex( &mutexFrame );
			if( frameHCopy.size > 0 ) { 
				imshow( "segmented", frameHCopy );
			}
			if( frameCopy.size > 0) {
				imshow( "original", frameCopy );
			}
			if(	frameGCopy.size > 0) {
				imshow( "blurred", frameGCopy );
			}
			if(	edgesCopy.size > 0) {
				imshow("edges", edgesCopy);
			}
			UnlockMutex( &mutexFrame );
		}
		else 
		{
			UnlockMutex( &mutexParameters );
		}
		
		/* provides processing time for the locator thread */
		/* necessary so that the windows remain visible */
		waitKey(200);
	}
}