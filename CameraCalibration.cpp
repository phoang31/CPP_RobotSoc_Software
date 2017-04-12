// CameraCalibration.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;
using namespace cv;
using namespace aruco;

const int FPS = 30;
const int WAITKEY_DELAY_MS = 1000 / FPS;
const int CAMERA_INDEX = 0;//webcamera. 1==external camera(usually)
const float CALIB_SQUARE_LENGTH = 0.026f;//2.6cm
//const float ARUCO_SQUARE_LENGTH = //TBD

//get known positions of corners on calibration chessboard
void generateKnownBoardPositions(Size boardSize, float squareEdgeLength, vector<Point3f>& corners)
{
	for (int i = 0; i < boardSize.height; i++)
	{
		for (int j = 0; j < boardSize.width; j++)
		{
			//loading in corner positions of chessboard. Z component is always zero with a flat calibration board
			corners.push_back(Point3f(j * squareEdgeLength, i * squareEdgeLength, 0.0f));
		}
	}
	return;
}

//gets detected chessboard corners, vector<Mat> images is a list of calibration images
void getChessboardCorners(vector<Mat> images, vector<vector<Point2f>>& allFoundCorners, bool displayResults = false)
{
	//iterates over vector of images
	for (vector<Mat>::iterator i = images.begin(); i != images.end(); i++)
	{
		vector<Point2f> pointBuffer;//stores locations of found corners
		bool found = findChessboardCorners(*i, Size(9, 6), pointBuffer, 
			CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);
		
		if (found == true)//found tells us if opencv discovered any chessboards in the images
		{
			allFoundCorners.push_back(pointBuffer);
		}

		//show found corners, if any
		if (displayResults == true)
		{
			drawChessboardCorners(*i, Size(9, 6), pointBuffer, found);
			imshow("Searching for corners...", *i);
			waitKey(0);
		}
	}
}

int main()
{
	char keyPress;
	Mat frame, drawFrame;

	//these two matrices are what we require to calibrate the camera later
	Mat cameraMatrix = Mat::eye(3,3, CV_64F);//starting with a 3x3 identity matrix
	Mat distortionMatrix;

	vector<Mat> savedImages;//for storing good calibration images

	vector<vector<Point2f>> markerCorners, rejectedCandidates;

	//attempt to open the capture device. return if failure
	VideoCapture cap(CAMERA_INDEX);
	if (!cap.isOpened())
	{
		cout << "Could not open camera " << CAMERA_INDEX << endl;
		return -1;
	}

	//keep the video stream running until we press a key
	while (true)
	{
		cap >> frame;//capture frame from camera

		if (frame.empty())//check if captured frame was valid, try again if not.
		{
			cout << "Invalid frame caught!" << endl;
			continue;
		}

		/*/////////////////////////////////////////////*/
		vector<Vec2f> foundPoints;
		bool found = false;

		found = findChessboardCorners(frame, Size(9, 6), foundPoints,
			CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);
		frame.copyTo(drawFrame);
		drawChessboardCorners(drawFrame, Size(9, 6), foundPoints, found);
		if (found)
		{
			imshow("Camera feed", drawFrame);
		}
		else
		{
			imshow("Camera feed", frame);
		}
		/*/////////////////////////////////////////////*/

		keyPress = (char)waitKey(WAITKEY_DELAY_MS);
		if (keyPress == 27)
		{
			break;//escape key pressed
		}
	}//capture device closes automatically on exit. no need to manually close it
    return 0;
}

