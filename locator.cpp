#include "project.h"

using namespace cv;
using namespace std;
using namespace Project;
// Function to ignore the found circles that have a longer or smaller radius than the limit
void DropCirclesByRadius( vector<Vec3f>& input, vector<Vec3f>& output , float radiusL, float radiusH );
// Locate the two marker circles
void LocateByDistance(const vector<Vec3f>& input, vector<Position>& markers , float distL, float distH, float minDistance );

/* This thread ontinously grabs a frame / test-image and process it. */
void* Project::LocatorThread(void* arg)
{
	cout << "LocatorThread thread starts "<< endl;
	clock_t start,stop;
	float counter = 0.0;
	VideoCapture cap(0); // open the default camera
	if(!cap.isOpened())  // check if we succeeded
	{
		throw runtime_error("Camera could not be opened");
		return -1;
	}
	while(1) 
	{
		start = clock();
		/* copy parameters to local variables so that 2 mutexes don't need to be locked in the same time */
		LockMutex( &mutexParameters );
		int cannyHCopy = cannyH;
		int cannyLCopy = cannyL;
		uchar saturationLimitCopy = saturationLimit;
		float radiusLimitHCopy = radiusLimitH;
		float radiusLimitLCopy = radiusLimitL;
		float distanceLimitHCopy = distanceLimitH;
		float distanceLimitLCopy = distanceLimitL;
		float minDistanceCopy = minDistance;
 
		UnlockMutex( &mutexParameters );

		/* lock mutex for image matrices */
		// load test image from camera
// 		frame = imread( "test_data/test.png" , 1);
		 cap >> frame; // get a new frame from camera
		 
		/** Color thresholding **/
		
		// Convert each frame from RGB to HSV
        cvtColor(frame, frameH, CV_BGR2HSV);
		// Threshold each pixel, the threshold is set for red colours.
		// awful sintax :(
        MatIterator_<Vec3b> it, end;	
        for( it = frameH.begin<Vec3b>(), end = frameH.end<Vec3b>(); it != end; ++it)
        {
			if(  !( ( ( (*it)[0]>170 && (*it)[0]<178 ) || ((*it)[0] > 0 && (*it)[0] < 10 )) && ( (*it)[1] > saturationLimitCopy ) ) )
            {
                (*it)[2] = 0;
                (*it)[1] = 0;
                (*it)[0] = 0;
            }
            else
            {
                (*it)[2] = 255;
                (*it)[1] = 0;
                (*it)[0] = 0;

            }
        }
        
        /** Convert the thresholded image to grayscale **/
        // (there is no CV_HSV2GRAY, so 2 steps are necessary)
        cvtColor(frameH, frameG, CV_HSV2BGR);
        cvtColor(frameG, frameG, CV_BGR2GRAY);
		
		/** Apply blur to the image so that not too many edges get detected **/
		
		GaussianBlur( frameG, frameG, Size(9, 9), 2, 2 );
		
		/** Detect edges and save the blurred image for visualization **/
		
		edges = frameG.clone();
		Canny(edges, edges, cannyLCopy, cannyHCopy, 3);
		
		/** Marker location **/
		
		vector<Vec3f> circles0; // this will contain all detected circles
		vector<Vec3f> circles;	// this will contain circles with appropiate radius
		
		// Apply the Hough Transform to find all the circles in the thresholded image
		HoughCircles( frameG, circles0, CV_HOUGH_GRADIENT, 1, 2, cannyHCopy, cannyLCopy, 3, 10 );
		
		// Drop the circles that don't have a sufficent radius
		DropCirclesByRadius( circles0, circles, radiusLimitLCopy, radiusLimitHCopy );
		cout << circles.size() << " suitable circles detected" << endl;
		
		LockMutex( &mutexMarkers );	// must be locked because Markes will be filled up here
		
		// Get the position of markers and calculate the average of too close detections, this also clears Markers
		LocateByDistance( circles, Markers, distanceLimitLCopy, distanceLimitHCopy, minDistanceCopy);
		vector<Position> MarkersCopy = Markers; // copy Markers so that we don't need to lock 2 mutexes at a time
		UnlockMutex( &mutexMarkers );
		
		// draw circles on original frame
		for( size_t i = 0; i < circles.size(); i++ )
		{
			Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
			int radius = cvRound(circles[i][2]);
			cout << "circle " << i << " radius: " << radius <<" center: ( "<< circles[i][0] << ";" << circles[i][1] << ") " << circles[i][2] << endl;
			circle( frame, center, 3, Scalar(0,255,0), -1, 8, 0 ); // circle center
			circle( frame, center, radius, Scalar(0,0,255), 3, 8, 0 ); // circle outline
		}
		
		// draw the located marker center on original frame
		for( int i = 0; i < MarkersCopy.size(); i++ )
		{
			cout << "Marker " << MarkersCopy[i].x << " " << MarkersCopy[i].y << endl;
			circle( frame, Point( MarkersCopy[i].x, MarkersCopy[i].y ), 3, Scalar(255,0,0), -1, 8, 0 ); // circle center
		}
		/* copy frames for the display thread
		 * This is necessary, because if frames were directly displayed, most of the time the unprocessed frames would be visible.
		 */
		LockMutex( &mutexFrame);
		
		frameCopy = frame.clone();
		frameGCopy = frameG.clone();
		edgesCopy = edges.clone();
		frameHCopy = frameH.clone();
		
		UnlockMutex( &mutexFrame );
		LockMutex( &mutexParameters );
		displayInitialDataAvailable = true;
		UnlockMutex( &mutexParameters );
		// Publish results with timestamp

        stop = clock () - start;
        start = clock ();
// 		counter +=0.1;
        cout << "Latency: " << ((float)stop)/CLOCKS_PER_SEC << "s" << endl;
	}
}

void DropCirclesByRadius( vector<Vec3f>& input, vector<Vec3f>& output , float radiusL, float radiusH )
{
	float radius;
	unsigned drop = 0;
	output.clear();
	// throw exception if not called properly
	if( radiusL >= radiusH ) 
		throw invalid_argument("Radius limits are equal or not in good order!");
	
	for( size_t i = 0; i < input.size(); i++ )
	{
			radius = input[i][2];
			if( radius >= radiusL && radius <= radiusH )
			{
				output.push_back( input[i] );
			}
	}
	cout << "Dropped " << drop << " circles by radius" << endl;
}

void LocateByDistance(const vector<Vec3f>& input, vector<Position>& markers , float distL, float distH, float minDistance )
{
	map<uint,uint> indexPairs;
	markers.clear();	// get rid of previous results
	if( distL <= 0.0 || distH <= 0.0 || distL >= distH )
		throw invalid_argument("Distance limit is invalid!");
	
	/* Compare each found circle center to all the others (combination) */
	// input[0] to input[1], input[2] ...
	// input[1] to input[2], input[3] ...
	// input[2] to input[3], input[4] ...
	// input[0-i] is compared to the others

	for( uint ii = 0; ii < input.size(); ii++ )
	{
		for( uint jj = ii+1; jj <= input.size(); jj++)
		{
			cout << "Comparing " << ii << " to " << jj << endl;
			float xdist = input[ii][0] - input[jj][0];
			float ydist = input[ii][1] - input[jj][1];
			float dist = sqrt( xdist*xdist + ydist*ydist );
			cout <<"Xdist: "<< input[ii][0] << "-" << input[jj][0] << " = "<< xdist << endl;
			cout <<"Ydist: "<< input[ii][1] << "-" << input[jj][1] << " = "<< ydist << endl;
			cout <<"Dist: " << dist << endl;
			/* if the distance of the two centers is OK, than a marker was found */
			/* store the indexes of the valid circle pair */
			if( dist <=distH && dist >= distL )
			{
				Position newLoc( (input[ii][0]+input[jj][0])/2.0, (input[ii][1]+input[jj][1])/2.0 );
				if( markers.size() > 0 )
				{
					for( uint kk =0; kk < markers.size() ; kk++ )
					{
						if( GetDistance(newLoc, markers[kk]) < minDistance )
						{
							/* Average results that are too close to each others and detect them as the same point */
							markers[kk].x = (markers[kk].x + newLoc.x)/2.0;
							markers[kk].y = (markers[kk].y + newLoc.y)/2.0;
						}
					}
				}
				else
				{
					markers.push_back( Position(newLoc.x,newLoc.y) );
				}
				cout << "Found marker at: ( " << markers[ markers.size()-1 ].x << " ; " << markers[ markers.size()-1].y << " ) " << endl ;
			}
		}
	}
	
}

















