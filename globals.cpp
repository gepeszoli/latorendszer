
#include "project.h"
using namespace cv;
using namespace std;
 
pthread_mutex_t Project::mutexParameters = PTHREAD_MUTEX_INITIALIZER ;
pthread_mutex_t Project::mutexFrame = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Project::mutexMarkers = PTHREAD_MUTEX_INITIALIZER;


Mat Project::edges; // container for edge detection result
Mat Project::frame; // container of image,
Mat Project::frameG; // the grayscale image, 
Mat Project::frameH; // the image represented in HSV

Mat Project::edgesCopy; // container for edge detection result
Mat Project::frameCopy; // container of image,
Mat Project::frameGCopy; // the grayscale image, 
Mat Project::frameHCopy; // the image represented in HSV

std::vector<Project::Position> Project::Markers; // this vector stores the position of the found markers

/* definitions for parameters */
bool Project::displayInitialDataAvailable = false;
bool Project::displayOn = true;
int Project::cannyH = 30;
int Project::cannyL = 10;
uchar Project::saturationLimit = (uchar)130;
float Project::radiusLimitH = 7.0;
float Project::radiusLimitL = 4.0;
float Project::distanceLimitH = 20.0;
float Project::distanceLimitL = 18.0;
float Project::minDistance = 5.0;

void Project::CheckError(int returnValue, const char *messageBeginning)
{
    char errorMsgBuf[200]; // this array would hold the error string returned by strerror_r
                           // check out: http://www.domaigne.com/blog/computing/pthreads-errors-and-errno/
    stringstream ss; // stringstream used to create exception messages
    if( returnValue < 0)
    {
        strerror_r( returnValue, errorMsgBuf, sizeof( errorMsgBuf ) / sizeof( errorMsgBuf[0] ) ); // get error message
        ss << messageBeginning << ": " << errorMsgBuf << " error code: " << returnValue; // write message to stringstream
        cout << ss.str() << endl;
        throw runtime_error( ss.str() );
    }
    return;
}

/** This function check for errors of the pthread functions and throws an exception if one is encountered */
void Project::CheckPthreadError(int returnValue, const char *messageBeginning)
{
    char errorMsgBuf[200]; // this array would hold the error string returned by strerror_r
                           // check out: http://www.domaigne.com/blog/computing/pthreads-errors-and-errno/
    stringstream ss; // stringstream used to create exception messages
    if( returnValue != 0)
    {
        strerror_r( returnValue, errorMsgBuf, sizeof( errorMsgBuf ) / sizeof( errorMsgBuf[0] ) ); // get error message
        ss << messageBeginning << ": " << errorMsgBuf; // write message to stringstream
        cout << ss.str() << endl;
        throw runtime_error( ss.str() );
    }
    return;
}
/* This function tries to lock a mutex and blocks if it cannot be locked */
void Project::LockMutex( pthread_mutex_t *mutexPtr)
{
    int rV = pthread_mutex_lock( mutexPtr );
    if( rV != 0) // if the lock failed
    {
		CheckPthreadError( rV, "LockMutex");
    }
}

void Project::UnlockMutex( pthread_mutex_t *mutexPtr)
{
    int rV = pthread_mutex_unlock( mutexPtr );
    if( rV != 0) // if the lock failed
    {
		CheckPthreadError( rV, "UnlockMutex");
    }
}


float Project::GetDistance( Position &a, Position& b )
{
	return sqrt( (a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y) );
}

int Project::writeToSocket(int fileDescriptor, const char* buffer, unsigned length) //protects against possible interruption of 'write'
{
    int left, written; // left: the number of bytes left to write written: the number of written bytes
    for(left = (int) length; left > 0; )
    {
        written = write(fileDescriptor, buffer, left);
        if(written <= 0)
        {
            if( written < 0 && errno == EINTR )
            {
                written = 0;
            }
            else
            {
                return -1; //signal error
            }
        }
        left -= written;
        buffer += written;
    }
    return length;
}
int Project::readFromSocket(int fileDescriptor, char* buffer, unsigned length) //protects against possible interruption of 'read'
{
    int bytesRead, left;
    left = length;
    while( left > 0 )
    {
        bytesRead = read(fileDescriptor, buffer, left);
        if(bytesRead < 0)
        {
            if( errno == EINTR)
            {
                bytesRead = 0;
            }
            else
            {
                return -1;
            }
        }
        else if(bytesRead == 0)
        {
            break;
        }
        left -= bytesRead;
        buffer += bytesRead;
    }
    return length-left;
}
 /*namespace project*/