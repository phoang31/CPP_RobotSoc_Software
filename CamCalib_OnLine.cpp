// CamCalib_OnLine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"



using namespace std;
using namespace cv;
using namespace aruco;



const float calibrationSquareDimension = 0.00246063f; //meters
const float arucoSquareDimension = 0.098425f; //meters
const Size chessboardDimensions = Size(6, 9);

void createKnownBoardPosition(Size boardSize, float squareEdgeLength, vector<Point3f>& corners)
{
	for (int i = 0; i < boardSize.height; i++)
	{
		for (int j = 0; j < boardSize.width; j++)
		{
			corners.push_back(Point3f(j * squareEdgeLength, i * squareEdgeLength, 0.0f));

		}
	}
}


void getChessboardCorners(vector<Mat> images, vector<vector<Point2f>>& allFoundCorners, bool showResults = false)
{
	for (vector<Mat>::iterator iter = images.begin(); iter != images.end(); iter++)
	{
		vector<Point2f> pointBuf;
		bool found = findChessboardCorners(*iter, Size(9, 6), pointBuf,CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);

		if (found)
		{
			allFoundCorners.push_back(pointBuf);
		}
		if (showResults)
		{
			drawChessboardCorners(*iter, Size(9, 6), pointBuf, found);
			imshow("Looking for Corners", *iter);
			waitKey(0);
		}
	}
}

int startWebcamMonitoring(const Mat& cameraMatrix, const Mat& distanceCoefficients, float arucoSquareDimensions)
{
	Mat frame;

	vector<int> markerIds;

	vector<vector<Point2f>> markerCorners, rejectedCandidates;
	aruco::DetectorParameters paremeters;
	Ptr < aruco::Dictionary> markerDictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50);
	
	VideoCapture vid(1);

	if (!vid.isOpened())
	{
		return -1;
	}
	
	namedWindow("Webcam", CV_WINDOW_AUTOSIZE);
	
	vector<Vec3d> rotationVectors, translationVectors;

	while (true)
	{
		if (!vid.read(frame))
			break;
		
		aruco::detectMarkers(frame, markerDictionary, markerCorners, markerIds);
		aruco::estimatePoseSingleMarkers(markerCorners, arucoSquareDimension, cameraMatrix, distanceCoefficients, rotationVectors, translationVectors);
		

		//draw markers////////////////////////////////////
		if (markerIds.size() > 0)
		{
			aruco::drawDetectedMarkers(frame, markerCorners, markerIds);
			cout << "Found IDs:" << endl;
			cout << Mat(markerIds) << endl;
		}
		else if (rejectedCandidates.size() > 0)
		{
			aruco::drawDetectedMarkers(frame, rejectedCandidates, noArray(), Scalar(100, 0, 255));
		}


		//draw axis//////////////////////////////////////
		for (int i = 0; i < markerIds.size(); i++)
		{
			aruco::drawAxis(frame, cameraMatrix, distanceCoefficients, rotationVectors[i], translationVectors[i], 0.09f);
			
		}

		/*
		//draw 3d cube in each marker
		for (unsigned int i = 0; i < markerIds.size(); i++)
		{
			CvDrawingUtils::draw3dCube()
		}
		*/


		imshow("Webcam", frame);
		if (waitKey(30) >= 0) break;
	}

	return 1;
}


void cameraCalibration(vector<Mat> calibrationImages, Size boardSize, float squareEdgeLength, Mat& cameraMatrix, Mat& distanceCoefficients)
{
	vector<vector<Point2f>> checkerboardImageSpacePoints;
	getChessboardCorners(calibrationImages, checkerboardImageSpacePoints, false);

	vector<vector<Point3f>> worldSpaceCornerPoints(1);

	createKnownBoardPosition(boardSize, squareEdgeLength, worldSpaceCornerPoints[0]);
	worldSpaceCornerPoints.resize(checkerboardImageSpacePoints.size(), worldSpaceCornerPoints[0]);

	vector<Mat> rVectors, tVectors;
	distanceCoefficients = Mat::zeros(8, 1, CV_64F);

	calibrateCamera(worldSpaceCornerPoints, checkerboardImageSpacePoints, boardSize, cameraMatrix, distanceCoefficients, rVectors, tVectors);
	
}

bool saveCameraCalibration(string name, Mat cameraMatrix, Mat distanceCoefficients)
{
	ofstream outStream(name);
	if (outStream)
	{
		uint16_t rows = cameraMatrix.rows;
		uint16_t columns = cameraMatrix.cols;

		outStream << rows << endl;
		outStream << columns << endl;

		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++)
			{
				double value = cameraMatrix.at<double>(r, c);
				outStream << value << endl;
			}
		}

		rows = distanceCoefficients.rows;
		columns = distanceCoefficients.cols;

		outStream << rows << endl;
		outStream << columns << endl;


		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++)
			{
				double value = distanceCoefficients.at<double>(r, c);
				outStream << value << endl;
			}
		}

		outStream.close();
		return true;
	}
	return false;
}

bool loadCameraCalibration(string name, Mat& cameraMatrix, Mat& distanceCoefficients)
{
	ifstream inStream(name);
	if (inStream)
	{
		uint16_t rows;
		uint16_t columns;

		inStream >> rows;
		inStream >> columns;

		cameraMatrix = Mat(Size(columns, rows), CV_64F);

		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++)
			{
				double read = 0.0f;
				inStream >> read;
				cameraMatrix.at<double>(r, c) = read;
				cout << cameraMatrix.at<double>(r, c) << "\n";
			}
		}
		//Distance Coefficients
		inStream >> rows;
		inStream >> columns;

		distanceCoefficients = Mat::zeros(rows, columns, CV_64F);

		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++)
			{
				double read = 0.0f;
				inStream >> read;
				distanceCoefficients.at<double>(r, c) = read;
				cout << distanceCoefficients.at<double>(r, c) << "\n";
			}
		}
		inStream.close();
		return true;
	}
	
	return false;

}

void cameraCalibrationProcess(Mat& cameraMatrix, Mat& distanceCoefficients)
{
	Mat frame;
	Mat drawToFrame;

	//Mat cameraMatrix = Mat::eye(3, 3, CV_64F);

	//Mat distanceCoefficients;

	vector<Mat> savedImages;

	vector<vector<Point2f>> markerCorners, rejectCandidates;

	VideoCapture vid(1);

	if (!vid.isOpened())
	{
		return ;
	}

	int framesPerSecond = 20;

	namedWindow("Webcam", CV_WINDOW_AUTOSIZE);

	while (true)
	{
		if (!vid.read(frame))
			break;
		vector<Vec2f> foundPoints;
		bool found = false;

		found = findChessboardCorners(frame, chessboardDimensions, foundPoints, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE); //to make it faster, we can also OR with "CV_CALIB_CB_FAST_CHECK"
		frame.copyTo(drawToFrame);
		drawChessboardCorners(drawToFrame, chessboardDimensions, foundPoints, found);

		if (found)
			imshow("Webcam", drawToFrame);
		else
			imshow("Webcam", frame);
		char character = waitKey(100 / framesPerSecond);

		switch (character)
		{
		case ' ':// Space key, 	//saving of the image
			if (found)
			{
				cout << "Saving Images" << endl;
				Mat temp;
				frame.copyTo(temp);
				savedImages.push_back(temp);

			}
			break;

		case 13: //Enter key, //start calibration
			if (savedImages.size() > 15)
			{
				cout << "Start Calibration" << endl;
				cameraCalibration(savedImages, chessboardDimensions, calibrationSquareDimension, cameraMatrix, distanceCoefficients);
				saveCameraCalibration("IloveCameraCalibration", cameraMatrix, distanceCoefficients);
			}
			break;

		case 27: //ESC key, //exit program
			{
			cout << "Exiting " << endl;
			return ;
			break;
			}
		}

	}
}

int main(int argv, char** argc)
{
	Mat cameraMatrix = Mat::eye(3, 3, CV_64F);

	Mat distanceCoefficients;

	cameraCalibrationProcess(cameraMatrix,distanceCoefficients);    //Use this first to calibrate the Camera/Lense
	//loadCameraCalibration("IloveCameraCalibration", cameraMatrix, distanceCoefficients);
	startWebcamMonitoring(cameraMatrix,distanceCoefficients,arucoSquareDimension);


	return 0;
}
