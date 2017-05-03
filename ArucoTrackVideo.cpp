// ArucoTrackVideo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;
using namespace cv;
using namespace aruco;

const int FPS = 30;
const int WAITKEY_DELAY_MS = 1000 / FPS;
const int CAMERA_INDEX = 1;//webcamera. 1==external camera(usually)
const float ARUCO_SQUARE_LENGTH = 0.076f;//7.6cm marker side length
const string CALIB_CONSTANTS_FILEPATH = "Camera_calibration_constants_projectCamera.txt";
const int CORNER_MARKER_ID_0 = 0;
const int CORNER_MARKER_ID_1 = 1;
const int ROBOT_MARKER_ID = 2;
const int MAX_PATH_BUFFER_LENGTH = 20;//must be greater than or equal to 2

const String CONTROLS_WINDOW_NAME = "Ball Tracking Controls";
const String CAMERA_WINDOW_NAME = "Camera Feed";
const int CANNY_UPPER_THRESH_MAX = 255;
const int CENTER_THRESH_MAX = 100;
const int BALL_MIN_RADIUS_MAX = 100;
const int BALL_MAX_RADIUS_MAX = 100;
const int BLUR_KERNEL_RADIUS_MAX = 10;

void calculateMarkerCenterAndAngle(vector<int> markerIDs, vector<vector<Point2f>> markerCorners, vector<Point2f> &markerCenters, vector<float> &angles);
int startWebcamMonitoring(const Mat cameraMatrix, const Mat distortionCoeffs, float arucoSquareSideLength);
bool readCameraCalibration(string fname, Mat& cameraMatrix, Mat& distortionCoefficients);
vector<Point2f> drawFieldCorners(Mat frame, vector<int> markerIDs, vector<Point2f> markerCenters);
int findRobotIndex(vector<int> markerIDs);
void drawPath(Mat frame, Point2f newLocation, deque<Point2f> &locationsBuffer);
Point2f trackCircles(Mat frame, int &cannyUpperThreshValue,
	int &centerDetectionThreshValue, int &minBallRadiusValue, int &maxBallRadiusValue, int &blurKernelRadiusValue);

//search through frames for aruco markers and draw orientation axes
int startWebcamMonitoring(const Mat cameraMatrix, const Mat distortionCoeffs, float arucoSquareSideLength)
{
	Mat grabbed, frame, gray;//frame to search for aruco markers and ball
	vector<Point2f> markerCenters, fieldCorners;
	Point2f circleCenter;
	vector<float> markerThetas;
	vector<int> markerIDs;
	vector<vector<Point2f>> markerCorners, rejectedCandidates;
	vector<Vec3f> circles;
	deque<Point2f> robotLocationsBuffer, ballLocationsBuffer;
	int robotIndex;
	int cannyUpperThreshValue = 100;
	int centerDetectionThreshValue = 24;
	int minBallRadiusValue = 9;
	int maxBallRadiusValue = 15;
	int blurKernelRadiusValue = 1;

	//control window
	namedWindow(CONTROLS_WINDOW_NAME, WINDOW_AUTOSIZE); // Create Window for trackbar
	createTrackbar("Canny Thresh.", CONTROLS_WINDOW_NAME, &cannyUpperThreshValue, CANNY_UPPER_THRESH_MAX);
	createTrackbar("Center Thresh.", CONTROLS_WINDOW_NAME, &centerDetectionThreshValue, CENTER_THRESH_MAX);
	createTrackbar("Ball min. radius", CONTROLS_WINDOW_NAME, &minBallRadiusValue, BALL_MIN_RADIUS_MAX);
	createTrackbar("Ball max. radius", CONTROLS_WINDOW_NAME, &maxBallRadiusValue, BALL_MAX_RADIUS_MAX);
	createTrackbar("Blur kernel radius", CONTROLS_WINDOW_NAME, &blurKernelRadiusValue, BLUR_KERNEL_RADIUS_MAX);

	//marker tracking
	Ptr<DetectorParameters> parameters = DetectorParameters::create();			//using default detector parameters for now

	Ptr<Dictionary> markerDictionary = getPredefinedDictionary(DICT_4X4_50);	//using small dictionary for fast results
																				
	VideoCapture cap(CAMERA_INDEX);												//attempt to open the capture device. return if failure
	if (!cap.isOpened())
	{
		cout << "Could not open camera " << CAMERA_INDEX << endl;
		return -1;
	}

	//keep the video stream running until we press a key
	while (true)
	{
		cap >> grabbed;//capture frame from camera

		if (grabbed.empty())//check if captured frame was valid, try again if not.
		{
			cout << "Invalid frame caught!" << endl;
			continue;
		}

		undistort(grabbed, frame, cameraMatrix, distortionCoeffs);//correct lens distortion on captured frame

		detectMarkers(frame, markerDictionary, markerCorners, markerIDs, parameters);

		//draw markers that are found
		if (markerIDs.size() > 0)
		{
			drawDetectedMarkers(frame, markerCorners, markerIDs);
			cout << "Found IDs:" << endl;
			cout << Mat(markerIDs) << endl;
		}

		//track the robot
		calculateMarkerCenterAndAngle(markerIDs, markerCorners, markerCenters, markerThetas);
		robotIndex = findRobotIndex(markerIDs);//check if the robot is in the frame, draw its path if so.
		if (robotIndex >= 0)
		{
			drawPath(frame, markerCenters[robotIndex], robotLocationsBuffer);
		}

		//track the ball
		circleCenter = trackCircles(frame, cannyUpperThreshValue, centerDetectionThreshValue, minBallRadiusValue, maxBallRadiusValue, blurKernelRadiusValue);
		if (circleCenter != Point2f(-1, -1))
		{
			drawPath(frame, circleCenter, ballLocationsBuffer);
		}

		//draw the field
		fieldCorners = drawFieldCorners(frame, markerIDs, markerCenters);
		if (fieldCorners.size() > 1)//print only if two corners were found
		{
			cout << "Field corners found = " << endl << fieldCorners << endl;
		}
		
		resize(frame, frame, Size(960, 720));
		imshow(CAMERA_WINDOW_NAME, frame);

		char keypress = waitKey(WAITKEY_DELAY_MS);
		if(keypress > 0)
		{
			break;
		}
	}
	return 0;//success
}

//reads calibration file and loads camera matrix and distortion vector data
bool readCameraCalibration(string fname, Mat& cameraMatrix, Mat& distortionCoefficients)
{
	ifstream instream(fname);
	string buffer;
	uint16_t lineCount = 0;
	uint16_t camRows = cameraMatrix.rows;//should be 3x3
	uint16_t camCols = cameraMatrix.cols;
	uint16_t distRows = distortionCoefficients.rows;//should be 1x5
	uint16_t distCols = distortionCoefficients.cols;
	uint16_t camElements = camRows*camCols;
	uint16_t distElements = distRows*distCols;
	uint16_t cr = 0 , cc = 0, dr = 0, dc = 0;//iterators

	if (instream)//check if we were able to open the file
	{
		//read contents of file
		while (getline(instream, buffer))
		{
			if (lineCount < camElements)//load camera matrix
			{
				if (cc < camCols)
				{
					cameraMatrix.at<double>(cr, cc) = atof(buffer.c_str());
					cc++;
				}
				else if(cr < camRows-1)
				{
					cc = 0;
					cr++;
					cameraMatrix.at<double>(cr, cc) = atof(buffer.c_str());
					cc++;
				}
			}
			else if (lineCount < (camElements + distElements))//load distortion coefficients. only the first row
			{
				if (dc < (distCols))
				{
					distortionCoefficients.at<double>(dr, dc) = atof(buffer.c_str());
					dc++;
				}
			}
			else//done loading
			{
				break;
			}
			lineCount++;
		}
		instream.close();

		cout << "Camera Matrix = " << endl << cameraMatrix << endl;
		cout << "Distortion Vector = " << endl << distortionCoefficients << endl;
		return true;
	}
	else
	{
		cout << "Could not open file: " << fname << endl;
		return false;
	}
}

//draws field lines on the screen from two markers in opposing corners and returns found corners locations, if any
vector<Point2f> drawFieldCorners(Mat frame, vector<int> markerIDs, vector<Point2f> markerCenters)
{
	int markerCount = markerIDs.size();
	bool corner1Found = false, corner2Found = false;
	int corner1ID, corner2ID;
	vector<Point2f> fieldCorners;
	//look for two particular markers. don't expect them to be in order
	if (markerCount > 0)
	{
		for (int i = 0; i < markerCount; i++)
		{
			if (markerIDs[i] == CORNER_MARKER_ID_0)
			{
				corner1Found = true;
				fieldCorners.push_back(markerCenters[i]);
				//cout << "Corner 1 found" << endl;
				continue;
			}
			if (markerIDs[i] == CORNER_MARKER_ID_1)
			{
				corner2Found = true;
				fieldCorners.push_back(markerCenters[i]);
				//cout << "Corner 2 found" << endl;
				continue;
			}
		}

		if (corner1Found && corner2Found)//draw the rectangle if both corner markers were found
		{
			rectangle(frame, fieldCorners[0], fieldCorners[1], Scalar(255, 255, 255), 2);
		}
	}
	return fieldCorners;
}

//finds center locations and orientations for each marker
void calculateMarkerCenterAndAngle(vector<int> markerIDs, vector<vector<Point2f>> markerCorners, vector<Point2f> &markerCenters, vector<float> &angles)
{
	Point2f markerCenter;
	double theta;

	angles.clear();			//clear the vectors to fill them up again
	markerCenters.clear();

	for (int i = 0; i < markerIDs.size(); i++)
	{
		markerCenter.x = (markerCorners[i][0].x + markerCorners[i][2].x) / 2.0f;//first corner is top left of marker, 3rd corner should be bottom right
		markerCenter.y = (markerCorners[i][0].y + markerCorners[i][2].y) / 2.0f;

		theta = -atan2((markerCorners[i][2].y - markerCorners[i][3].y), (markerCorners[i][2].x - markerCorners[i][3].x))*180.0f / CV_PI;

		markerCenters.push_back(markerCenter);
		angles.push_back(theta);

		/*
		cout << "markerIDs = " << endl << markerIDs[i] << endl;
		cout << "markerCenters = " << endl << markerCenters[i] << endl;
		cout << "angles = " << endl << angles[i] << endl;
		*/
	}
	
}

//finds robot by markerID, returns index within markerIDs, markerCenters, angles
int findRobotIndex(vector<int> markerIDs)
{
	for (int i = 0; i < markerIDs.size(); i++)
	{
		if (markerIDs[i] == ROBOT_MARKER_ID)
		{
			return i;
		}
	}
	return -1;
}

//draws the path of a tracked object on the frame using a fifo buffer of previous points
void drawPath(Mat frame, Point2f newLocation, deque<Point2f> &locationsBuffer)
{
	locationsBuffer.push_back(newLocation);//add newest item to list
	if (locationsBuffer.size() > MAX_PATH_BUFFER_LENGTH)
	{
		locationsBuffer.pop_front();//start removing oldest items
		for (int i = 0; i < (locationsBuffer.size() - 1); i++)//buffer size should already be >= 2
		{
			line(frame, locationsBuffer[i], locationsBuffer[i + 1], Scalar(0,0,((64*i) % 256)), 3);
		}
	}
}


Point2f trackCircles(Mat frame, int &cannyUpperThreshValue,
	int &centerDetectionThreshValue, int &minBallRadiusValue, int &maxBallRadiusValue, int &blurKernelRadiusValue)
{
	vector<Vec3f> circles;
	Mat gray;
	cvtColor(frame, gray, COLOR_BGR2GRAY);
	medianBlur(gray, gray, blurKernelRadiusValue * 2 + 1);//apply a blur to the gray frame to help find circles

	HoughCircles(gray, circles, HOUGH_GRADIENT, 1,
		gray.rows / 16,//alter this to detect circles with different distances from eachother
		cannyUpperThreshValue + 1, centerDetectionThreshValue + 1, minBallRadiusValue, maxBallRadiusValue);//alter last two parameters for min/max ball size

	if (circles.size() > 0)
	{
		Point2f c = { circles[0][0], circles[0][1] };
		circle(gray, Point(c.x, c.y), circles[0][2], Scalar(0, 0, 255), 3, LINE_AA);
		imshow("blurred grayscale", gray);
		circle(frame, Point(c.x, c.y), 2, Scalar(0, 255, 0), 3, LINE_AA);
		cout << "Ball found @ : " << c << endl;
		return c;
	}
	else
	{
		return Point2f(-1, -1);//no circles found
	}
}

int main()
{
	Mat cameraMatrix(3, 3, CV_64F);
	Mat distortionCoefficients(1,5, CV_64F);
	readCameraCalibration(CALIB_CONSTANTS_FILEPATH, cameraMatrix, distortionCoefficients);
	startWebcamMonitoring(cameraMatrix, distortionCoefficients, ARUCO_SQUARE_LENGTH);
    return 0;
}

