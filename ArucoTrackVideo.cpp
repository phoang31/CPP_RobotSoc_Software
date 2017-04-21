// ArucoTrackVideo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;
using namespace cv;
using namespace aruco;

const int FPS = 30;
const int WAITKEY_DELAY_MS = 1000 / FPS;
const int CAMERA_INDEX = 0;//webcamera. 1==external camera(usually)
const float ARUCO_SQUARE_LENGTH = 0.076f;//7.6cm marker side length
const string CALIB_CONSTANTS_FILEPATH = "Camera_calibration_constants.txt";


//search through frames for aruco markers and draw orientation axes
int startWebcamMonitoring(const Mat cameraMatrix, const Mat distortionCoeffs, float arucoSquareSideLength)
{
	Mat frame;//frame to search for aruco markers

	vector<int> markerIDs;
	vector<vector<Point2f>> markerCorners, rejectedCandidates;

	Ptr<DetectorParameters> parameters = DetectorParameters::create();//using default detector parameters for now

	Ptr<Dictionary> markerDictionary = getPredefinedDictionary(DICT_4X4_50);//using small dictionary for fast results
	vector<Vec3d>  rotationVectors, translationVectors;//stores orientation of aruco markers
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
		detectMarkers(frame, markerDictionary, markerCorners, markerIDs, parameters);
		estimatePoseSingleMarkers(markerCorners, arucoSquareSideLength, cameraMatrix,
			distortionCoeffs, rotationVectors, translationVectors);

		//draw markers
		if (markerIDs.size() > 0)
		{
			drawDetectedMarkers(frame, markerCorners, markerIDs);
			cout << "Found IDs:" << endl;
			cout << Mat(markerIDs) << endl;
		}
		else if (rejectedCandidates.size() > 0)
		{
			drawDetectedMarkers(frame, rejectedCandidates, noArray(), Scalar(100, 0, 255));
		}

		//draw axes on markers
		for (int i = 0; i < markerIDs.size(); i++)
		{
			//cout << "rvec = " << i << endl << rotationVectors[i] << endl;
			//cout << "tvec = " << i << endl << translationVectors[i] << endl;
			drawAxis(frame, cameraMatrix, distortionCoeffs, rotationVectors[i], translationVectors[i], arucoSquareSideLength);
		}

		imshow("Camera feed", frame);

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

int main()
{
	Mat cameraMatrix(3, 3, CV_64F);
	Mat distortionCoefficients(1,5, CV_64F);
	readCameraCalibration(CALIB_CONSTANTS_FILEPATH, cameraMatrix, distortionCoefficients);
	startWebcamMonitoring(cameraMatrix, distortionCoefficients, ARUCO_SQUARE_LENGTH);
    return 0;
}

