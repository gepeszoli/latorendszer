#ifndef PROJECT_H
#define PROJECT_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <ctime>
#include <stdexcept>

#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <resolv.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include <signal.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace Project{

// ================================================================
//				Global classes, structs, unions
// ================================================================

class Position // utility class, simply for coordinate pairs 	
{
public:
	float x,y;
	Position(float X=0.0, float Y=0.0):x(X),y(Y)
	{}
};

class InputCommand
{
public:
	int code;
	unsigned char data[4];
	InputCommand(int code = -1 ):code(code)
	{
		for(int i=0; i<4; i++) data[i]=0;
	}
};

// ================================================================
//				Global variables
// ================================================================	

/* Mutexes for synchronizations */
extern pthread_mutex_t mutexParameters;	// to protect process wide parameters
extern pthread_mutex_t mutexFrame;		// to protect image data (without the need to copy it)
extern pthread_mutex_t mutexMarkers;	// to protect the detection result

/* cv::Mat matrices, containing the image data */
extern cv::Mat edges; // container for edge detection result
extern cv::Mat frame; // container of image,
extern cv::Mat frameG; // the grayscale image, 
extern cv::Mat frameH; // the image represented in HSV

/* copies of original images, for display */
extern cv::Mat edgesCopy; // container for edge detection result
extern cv::Mat frameCopy; // container of image,
extern cv::Mat frameGCopy; // the grayscale image, 
extern cv::Mat frameHCopy; // the image represented in HSV

/* parameters for the operation of the program */
extern bool displayInitialDataAvailable ; // to check if the very first frame is captured and processed, before it is tried to be displayes
extern bool displayOn;	// global variable to control whether the frames and the results of processing are shown
extern int cannyH ;		// limits for Canny edge detector 
extern int cannyL ;
extern uchar saturationLimit ;	// saturation threshold for color thresholding
extern float radiusLimitH ;		// maximum allowed circle radius for detected circles
extern float radiusLimitL;		// minimum required circle radius for detected circles
extern float distanceLimitH;	// upper distance threshold between two circle markers that are recognized as a marker
extern float distanceLimitL;	// upper distance threshold between two circle markers that are recognized as a marker
extern float minDistance;		// minimum required distance between two detected marker center locations that are considered as separate markers

/* container of detection results */
extern std::vector<Position> Markers; // this vector stores the position of the found markers

// ================================================================
//				Global functions
// ================================================================	

void LockMutex( pthread_mutex_t *mutexPtr);		// convenience function to lock a pthread_mutex

void UnlockMutex( pthread_mutex_t *mutexPtr);	// convenience function to unlock a pthread_mutex

float GetDistance( Position &a, Position& b ); // returns the distance between two Position objects

void CheckError(int returnValue, const char *messageBeginning); 				// checks for errors of the system calls and throws an exception if one is encountered */

void CheckPthreadError(int returnValue, const char *messageBeginning); 			// checks for errors of the pthread library and throws an exception if one is encountered */

void InitalizeBroadcastSocket(int& socketFileDescriptor, sockaddr_in& address); // initializes socket for UDP broadcast

void InitalizeCommandSocket(int& socketFileDescriptor, sockaddr_in& address);	// initializes socket for TCP command transfer

int writeToSocket(int fileDescriptor, const char* buffer, unsigned length); 			// a function that writes to a socket and protects against 
																				// possible interruptions of the 'write' system call

int readFromSocket(int fileDescriptor, char* buffer, unsigned length); 		// a function that reads from a socket and protects against 
																			// possible interruptions of the 'read' system call

int Calibrate();	// this function performs the calibration see: http://docs.opencv.org/doc/tutorials/calib3d/camera_calibration/camera_calibration.html

void HandleClient( int clientSocket );	// This function processes the commands sent to the program 

// ================================================================
//				Thread functions
// ================================================================	

void* LocatorThread(void* arg); 		// thread for marker detection
void* DisplayThread(void* arg);			// thread for visualizing result
void* InputCommandThread(void* arg); 	// thread for processing user input to the program 
void* BroadcasterThread(void* arg);		// thread for broadcasting the found marker locations



// ================================================================
//				Command codes
// ================================================================	

const int CmdInvalid = -1;
const int CmdSetDisplayOn = 1;
const int CmdSetDisplayOff = 2;
const int CmdSetSaturationLimit = 3;

	
}
#endif
