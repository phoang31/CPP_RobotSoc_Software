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
//const float ARUCO_SQUARE_LENGTH = 0.076f;//7.6cm marker side length
const string CALIB_CONSTANTS_FILEPATH = "Camera_calibration_constants.txt";

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

//calculates camera matrix and distortion vector from calibration images
void cameraCalibration(vector<Mat> calibrationImages, Size boardSize, float squareSideLength,
	Mat& CameraMatrix, Mat& distortionMatrix)
{
	//coordinates detected on calibration images
	vector<vector<Point2f>> checkerBoardImageSpacePoints;
	getChessboardCorners(calibrationImages, checkerBoardImageSpacePoints, false);

	//create known board positions for all images
	vector<vector<Point3f>> worldSpaceCornerPoints(1);

	generateKnownBoardPositions(boardSize, squareSideLength, worldSpaceCornerPoints[0]);
	//resize matrix and copy values over
	worldSpaceCornerPoints.resize(checkerBoardImageSpacePoints.size(), worldSpaceCornerPoints[0]);

	//radial and tangential vectors
	vector<Mat> rVectors, tVectors;
	//distortion coefficients
	distortionMatrix = Mat::zeros(8, 1, CV_64F);

	//calibrate the camera
	calibrateCamera(worldSpaceCornerPoints, checkerBoardImageSpacePoints, boardSize,
		CameraMatrix, distortionMatrix, rVectors, tVectors);



}

//writes calibration constants to a text file
bool saveCameraCalibration(string name, Mat cameraMatrix, Mat distortionCoefficients)
{
	ofstream outstream(name);//create output file stream
	if(outstream)
	{
		//write 
		uint16_t rows = cameraMatrix.rows;
		uint16_t cols = cameraMatrix.cols;

		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < cols; c++)
			{
				double value = cameraMatrix.at<double>(r, c);
				outstream << value << endl;//writing matrix elements to file
			}
		}

		rows = distortionCoefficients.rows;
		cols = distortionCoefficients.cols;

		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < cols; c++)
			{
				double value = distortionCoefficients.at<double>(r, c);
				outstream << value << endl;//writing matrix elements to file
			}
		}
		outstream.close();
		return true;
	}
	else
	{
		return false;
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

		//handle keypresses
		keyPress = (char)waitKey(WAITKEY_DELAY_MS);
		switch (keyPress)
		{
		case ' '://spacebar saves image when we have a good frame
			if (found)
			{
				Mat tempFrame;
				frame.copyTo(tempFrame);
				savedImages.push_back(tempFrame);
				cout << "Saved image: " << savedImages.size() << endl;
			}
			else
			{
				cout << "Chessboard not found in image!" << endl;
			}
			break;
		case 13://enter key begins calibration once we have enough images
			if (savedImages.size() >= 15)
			{
				cout << "Calculating calibration constants. Please wait." << endl;
				cameraCalibration(savedImages, Size(9, 6), CALIB_SQUARE_LENGTH, cameraMatrix, distortionMatrix);
				cout << "Camera matrix = " << endl << cameraMatrix << endl;
				cout << "Distortion coefficients = " << endl << distortionMatrix << endl;
				bool savedResult = saveCameraCalibration(CALIB_CONSTANTS_FILEPATH, cameraMatrix, distortionMatrix);
				if (savedResult == true)
				{
					cout << "Saved camera calibration constants to text file." << endl;
				}
				else
				{
					cout << "Failed to save calibration constants to text file!" << endl;
				}
			}
			else
			{
				cout << "Not enough images for calibration. Need at least 15." << endl;
			}
			return 0;
			break;
		case 27://escape key exits program
			cout << "Exiting" << endl;
			return 0;
			break;
		default://do nothing for other keypresses
			break;

		};
	}//capture device closes automatically on exit. no need to manually close it
	 /*/////////////////////////////////////////////*/
    return 0;
}

