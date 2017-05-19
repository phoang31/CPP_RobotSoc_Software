// CamCalib_OnLine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;
using namespace cv;
using namespace aruco;

//TCP BEGINS **********************************************************************
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 6											//buffer length 
#define DEFAULT_PORT "5002"											//port number
#define DEFAULT_IP "192.168.2.9"                                    //Robot IP

bool TCP_IP() //argument 1 is servername, argument 2 is server ip address
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	char *sendbuf;   //points to userinput
	char userinput[1] = { 'd' };
	char *exitmessage = "Exiting";
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(DEFAULT_IP, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {															//EXIT IF FAILED TO CONNECT
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}
	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {									//EXIT IF FAILED TO CREATE A SOCKET
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}
		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}
	freeaddrinfo(result);    //FREE RESULT POINTER DATA
	if (ConnectSocket == INVALID_SOCKET) {				//EXIT IF UNABLE TO CONNECT TO SERVER
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}
	
	printf("Connected to server. Can start sending data.\n");
	do
	{

		printf("Enter a command: d,b,r,l for action: ");

		cin >> userinput[0];

		sendbuf = userinput;			//need to relace userinput by the command from aruco, we can make a function to do this

		cout << "Received input\n";

		// Send an initial buffer

		iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);

		if (iResult == SOCKET_ERROR) {

			printf("send failed with error: %d\n", WSAGetLastError());

			closesocket(ConnectSocket);

			WSACleanup();

			return 1;

		}

		printf("Bytes Sent: %ld\n", iResult);

		if (*sendbuf == 'q' || *sendbuf == 'Q')

		{

			cout << *exitmessage << endl;

			return 1;

		}

		/*

		//Receiving data

		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);

		if (iResult > 0)

		{

		printf("Bytes received: %d\n", iResult);

		cout << recvbuf << endl;

		}

		else if (iResult == 0)

		printf("Connection closed\n");

		else

		printf("recv failed with error: %d\n", WSAGetLastError());

		*/

	} while (iResult > 0);





	// shutdown the connection since no more data will be sent

	iResult = shutdown(ConnectSocket, SD_SEND);

	if (iResult == SOCKET_ERROR) {

		printf("shutdown failed with error: %d\n", WSAGetLastError());

		closesocket(ConnectSocket);

		WSACleanup();

		return 1;

	}



	// cleanup

	closesocket(ConnectSocket);
	WSACleanup();
	return 0;

}

// TCP/IP end**********************************************************************
const float calibrationSquareDimension = 0.00246063f; //meters
const float arucoSquareDimension = 0.098425f; //meters
const Size chessboardDimensions = Size(6, 9);

//Additional (for ball detection and tracking)********************************************************************

//Object/ball detected coordinate Global variables
int x = 0, y = 0;

//Aruco MarkerTheta
Mat frame;
vector<int> markerThetas;
vector<Point2f> markerCenters;
int RobotTheta;
int RobotIndex; //initialized as no robot (-1)
Point2f markerCenter;
const int CORNER_MARKER_ID_0 = 0;
const int CORNER_MARKER_ID_1 = 1;
const int ROBOT_MARKER_ID = 2;

//initial HSV filter values (will be changed using trackbars or by selection using pointer to create a rectangle)
int H_MIN = 0;
int H_MAX = 256;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;
//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
//names that will appear at the top of each window
const string windowName = "Original Image";
const string windowName1 = "HSW Image";
const string windowName2 = "Thresholded Image";
const string windowName3 = "After Morpholigical Operations";
const string trackbarWindowName = "Trackbars";

bool calibrationMode;//used for showing debugging windows, trackbars etc.

bool mouseIsDragging;//used for showing a rectangle on screen as user clicks and drags mouse
bool mouseMove;
bool rectangleSelected;
cv::Point initialClickPoint, currentMousePoint; //keep track of initial point clicked and current position of mouse
cv::Rect rectangleROI; //this is the ROI that the user has selected
vector<int> H_ROI, S_ROI, V_ROI;// HSV values from the click/drag ROI region stored in separate vectors so that we can sort them easily

								// TCP/IP start*******************************************************************
#define WIN32_LEAN_AND_MEAN

								// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib

// Color Selection tracking********************************************************
void clickAndDrag_Rectangle(int event, int x, int y, int flags, void* param) {
	//only if calibration mode is true will we use the mouse to change HSV values
	if (calibrationMode == true) {
		//get handle to video feed passed in as "param" and cast as Mat pointer
		Mat* videoFeed = (Mat*)param;

		if (event == CV_EVENT_LBUTTONDOWN && mouseIsDragging == false)
		{
			//keep track of initial point clicked
			initialClickPoint = cv::Point(x, y);
			//user has begun dragging the mouse
			mouseIsDragging = true;
		}
		/* user is dragging the mouse */
		if (event == CV_EVENT_MOUSEMOVE && mouseIsDragging == true)
		{
			//keep track of current mouse point
			currentMousePoint = cv::Point(x, y);
			//user has moved the mouse while clicking and dragging
			mouseMove = true;
		}
		/* user has released left button */
		if (event == CV_EVENT_LBUTTONUP && mouseIsDragging == true)
		{
			//set rectangle ROI to the rectangle that the user has selected
			rectangleROI = Rect(initialClickPoint, currentMousePoint);

			//reset boolean variables
			mouseIsDragging = false;
			mouseMove = false;
			rectangleSelected = true;
		}

		if (event == CV_EVENT_RBUTTONDOWN) {
			//user has clicked right mouse button
			//Reset HSV Values
			H_MIN = 0;
			S_MIN = 0;
			V_MIN = 0;
			H_MAX = 255;
			S_MAX = 255;
			V_MAX = 255;

		}
		if (event == CV_EVENT_MBUTTONDOWN) {

			//user has clicked middle mouse button
			//enter code here if needed.
		}
	}

}

void recordHSV_Values(cv::Mat frame, cv::Mat hsv_frame) {

	//save HSV values for ROI that user selected to a vector
	if (mouseMove == false && rectangleSelected == true) {

		//clear previous vector values
		if (H_ROI.size()>0) H_ROI.clear();
		if (S_ROI.size()>0) S_ROI.clear();
		if (V_ROI.size()>0)V_ROI.clear();
		//if the rectangle has no width or height (user has only dragged a line) then we don't try to iterate over the width or height
		if (rectangleROI.width<1 || rectangleROI.height<1) cout << "Please drag a rectangle, not a line" << endl;
		else {
			for (int i = rectangleROI.x; i<rectangleROI.x + rectangleROI.width; i++)
			{
				//iterate through both x and y direction and save HSV values at each and every point
				for (int j = rectangleROI.y; j<rectangleROI.y + rectangleROI.height; j++)
				{
					//save HSV value at this point
					H_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[0]);
					S_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[1]);
					V_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[2]);
				}
			}
		}
		//reset rectangleSelected so user can select another region if necessary
		rectangleSelected = false;
		//set min and max HSV values from min and max elements of each array

		if (H_ROI.size()>0)
		{
			//NOTE: min_element and max_element return iterators so we must dereference them with "*"
			H_MIN = *std::min_element(H_ROI.begin(), H_ROI.end());
			H_MAX = *std::max_element(H_ROI.begin(), H_ROI.end());
			cout << "MIN 'H' VALUE: " << H_MIN << endl;
			cout << "MAX 'H' VALUE: " << H_MAX << endl;
		}
		if (S_ROI.size()>0)
		{
			S_MIN = *std::min_element(S_ROI.begin(), S_ROI.end());
			S_MAX = *std::max_element(S_ROI.begin(), S_ROI.end());
			cout << "MIN 'S' VALUE: " << S_MIN << endl;
			cout << "MAX 'S' VALUE: " << S_MAX << endl;
		}
		if (V_ROI.size()>0)
		{
			V_MIN = *std::min_element(V_ROI.begin(), V_ROI.end());
			V_MAX = *std::max_element(V_ROI.begin(), V_ROI.end());
			cout << "MIN 'V' VALUE: " << V_MIN << endl;
			cout << "MAX 'V' VALUE: " << V_MAX << endl;
		}

	}

	if (mouseMove == true) {
		//if the mouse is held down, we will draw the click and dragged rectangle to the screen
		rectangle(frame, initialClickPoint, cv::Point(currentMousePoint.x, currentMousePoint.y), cv::Scalar(0, 255, 0), 1, 8, 0);
	}


}

string intToString(int number) {


	std::stringstream ss;
	ss << number;
	return ss.str();
}

void drawObject(int x, int y, Mat &frame)
{
	//use some of the openCV drawing functions to draw crosshairs
	//on your tracked image!

	//added 'if' and 'else' statements to prevent
	//memory errors from writing off the screen (ie. (-25,-25) is not within the window!)

	circle(frame, Point(x, y), 20, Scalar(0, 255, 0), 2);
	if (y - 25>0)
		line(frame, Point(x, y), Point(x, y - 25), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(x, 0), Scalar(0, 255, 0), 2);
	if (y + 25<FRAME_HEIGHT)
		line(frame, Point(x, y), Point(x, y + 25), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(x, FRAME_HEIGHT), Scalar(0, 255, 0), 2);
	if (x - 25>0)
		line(frame, Point(x, y), Point(x - 25, y), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(0, y), Scalar(0, 255, 0), 2);
	if (x + 25<FRAME_WIDTH)
		line(frame, Point(x, y), Point(x + 25, y), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(FRAME_WIDTH, y), Scalar(0, 255, 0), 2);

	putText(frame, intToString(x) + "," + intToString(y), Point(x, y + 30), 1, 1, Scalar(0, 255, 0), 2);

}

void on_trackbar(int, void*)
{
	//does nothing for now
}

void createTrackbars()
{
	//create window for trackbars
	namedWindow("trackbars", 0);
	//create memory to store trackbar name on window
	char TrackbarName[50];
	sprintf(TrackbarName, "H_MIN", H_MIN);
	sprintf(TrackbarName, "H_MAX", H_MAX);
	sprintf(TrackbarName, "S_MIN", S_MIN);
	sprintf(TrackbarName, "S_MAX", S_MAX);
	sprintf(TrackbarName, "V_MIN", V_MIN);
	sprintf(TrackbarName, "V_MAX", V_MAX);

	//create trackbars and insert them into window
	createTrackbar("H_MIN", "trackbars", &H_MIN, H_MAX, on_trackbar);
	createTrackbar("H_MAX", "trackbars", &H_MAX, H_MAX, on_trackbar);
	createTrackbar("S_MIN", "trackbars", &S_MIN, S_MAX, on_trackbar);
	createTrackbar("S_MAX", "trackbars", &S_MAX, S_MAX, on_trackbar);
	createTrackbar("V_MIN", "trackbars", &V_MIN, V_MAX, on_trackbar);
	createTrackbar("V_MAX", "trackbars", &V_MAX, V_MAX, on_trackbar);
}

void morphOps(Mat &thresh) {

	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle

	Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3, 3));
	//dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8, 8));

	erode(thresh, thresh, erodeElement);
	erode(thresh, thresh, erodeElement);


	dilate(thresh, thresh, dilateElement);
	dilate(thresh, thresh, dilateElement);



}

void trackFilteredObject(int &x, int&y, Mat threshold, Mat &cameraFeed)
{
	Mat temp;
	threshold.copyTo(temp);
	//these two vectors needed for output of findContours
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	//use moments method to find our filtered object
	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0)
	{
		int numObjects = hierarchy.size();
		//if number of objects greater than 10, probably a noisy filter
		if (numObjects < 10)
		{
			for (int index = 0; index >= 0; index = hierarchy[index][0])
			{
				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				if (area > 400 && area<FRAME_HEIGHT*FRAME_WIDTH / 1.5 && area>refArea)
				{
					printf("%f \n", area);
					x = moment.m10 / area;
					y = moment.m01 / area;
					objectFound = true;
				}
				else objectFound = false;
			}
			if (objectFound == true)
			{
				putText(cameraFeed, "Tracking Object", Point(0, 50), 2, 1, Scalar(0, 255, 0), 2);
				//draw object location on screen
				drawObject(x, y, cameraFeed);
			}
			else putText(cameraFeed, "Too Much Noise! Adjust Filter", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
		}
	}

}

//END of additional function ***************************************************************************

//OpenCV Camera Calibration*****************************************************************************
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
		bool found = findChessboardCorners(*iter, Size(9, 6), pointBuf, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);

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
		return;
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
				saveCameraCalibration("IloveCameraCalibration_2", cameraMatrix, distanceCoefficients);
			}
			break;

		case 27: //ESC key, //exit program
		{
			cout << "Exiting " << endl;
			return;
			break;
		}
		}

	}
}

//Roberts codes ************ 

void calculateMarkerCenterAndAngle(vector<int> markerIDs, vector<vector<Point2f>> markerCorners, vector<Point2f> &markerCenters, vector<int> &angles)
{
	//Point2f markerCenter;
	//int theta;

	angles.clear();			//clear the vectors to fill them up again

	markerCenters.clear();

	for (int i = 0; i < markerIDs.size(); i++)
	{
		markerCenter.x = (markerCorners[i][0].x + markerCorners[i][2].x) / 2.0f;//first corner is top left of marker, 3rd corner should be bottom right
		markerCenter.y = (markerCorners[i][0].y + markerCorners[i][2].y) / 2.0f;

		RobotTheta = -atan2((markerCorners[i][2].y - markerCorners[i][3].y), (markerCorners[i][2].x - markerCorners[i][3].x))*180.0f / CV_PI;

		markerCenters.push_back(markerCenter);

		angles.push_back(RobotTheta);

		/*
		cout << "markerIDs = " << endl << markerIDs[i] << endl;
		cout << "markerCenters = " << endl << markerCenters[i] << endl;
		cout << "angles = " << endl << angles[i] << endl;
		*/
	}
}

int startWebcamMonitoring(const Mat& cameraMatrix, const Mat& distanceCoefficients, float arucoSquareDimensions)
{
	Mat frame, res; //Captured frame in Original: Frame, Undistorted: res (result)

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
			continue;
		//break;
		undistort(frame, res, cameraMatrix, distanceCoefficients);
		aruco::detectMarkers(frame, markerDictionary, markerCorners, markerIds);
		//aruco::detectMarkers(res, markerDictionary, markerCorners, markerIds); //undistorted
		aruco::estimatePoseSingleMarkers(markerCorners, arucoSquareDimension, cameraMatrix, distanceCoefficients, rotationVectors, translationVectors);



		//draw markers////////////////////////////////////
		if (markerIds.size() > 0)
		{
			aruco::drawDetectedMarkers(frame, markerCorners, markerIds);
			//aruco::drawDetectedMarkers(res, markerCorners, markerIds);   //aruco marker detection on undistorted capture
			cout << "Found IDs:" << endl;
			cout << Mat(markerIds) << endl;
			//aruco::drawMarker(markerDictionary, markerIds[0], 100, frame, 1); //prints detected aruco marker
		}
		else if (rejectedCandidates.size() > 0)
		{
			aruco::drawDetectedMarkers(frame, rejectedCandidates, noArray(), Scalar(100, 0, 255));
			//aruco::drawDetectedMarkers(res, rejectedCandidates, noArray(), Scalar(100, 0, 255));    //aruco marker detection on undistorted capture
		}


		//draw axis//////////////////////////////////////
		for (int i = 0; i < markerIds.size(); i++)
		{
			aruco::drawAxis(frame, cameraMatrix, distanceCoefficients, rotationVectors[i], translationVectors[i], 0.09f);
			//aruco::drawAxis(res, cameraMatrix, distanceCoefficients, rotationVectors[i], translationVectors[i], 0.09f); //aruco marker draw axis on undistorted capture
		}

		//draw rectangle between two points
		if (markerIds.size() > 1)
		{
			//need to designate marker ID 1 to markerCorner[0][0]
			int width = abs((int)markerCorners[1][0].x - (int)markerCorners[0][0].x);
			cout << "Width" << width << endl;
			int height = abs((int)markerCorners[1][0].y - (int)markerCorners[0][0].y);
			cout << "Height" << height << endl;

			cv::Rect rect((int)markerCorners[0][0].x, (int)markerCorners[0][0].y, width, height);
			cv::Point pt1((int)markerCorners[0][0].x, (int)markerCorners[0][0].y);
			cv::Point pt2((int)markerCorners[1][0].x, (int)markerCorners[1][0].y);

			cv::rectangle(frame, pt1, pt2, cv::Scalar(0, 255, 0)); //create rect on regular capture
																   //cv::rectangle(res, pt1, pt2, cv::Scalar(0, 255, 0)); //create rect on undistorted capture  
		}

		//Prints out the coordinate of the corner origin of the aruco marker  
		for (int a = 0; a < markerIds.size(); a++)
		{
			cout << "marker : " << endl;
			cout << markerIds[a] << endl;
			cout << (int)markerCorners[a][0].x << endl; //to separate the coordinates
			cout << (int)markerCorners[a][0].y << endl;
		}


		//Prints out and finds the middle of the two aruco markers, making it the new origin of the coordinate


		imshow("Webcam", frame);
		//imshow("Undistorted", res);
		cout << "END " << endl;
		cout << endl;
		if (waitKey(30) >= 0) break;
	}

	return 1;
}

int findRobotIndex(vector<int> markerIDs)
{
	for (int i = 0; i < markerIDs.size(); i++)
	{
		if (markerIDs[i] == ROBOT_MARKER_ID)
		{
			RobotIndex = i;
			return i;
		}
	}
	return -1;
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

//End of Roberts codes *****


//finds center locations and orientations for Robot's Aruco marker
void calculateRobotMarkerCenterAndAngle(const Mat& frame, vector<int> markerIDS)
{
	int robIndex = findRobotIndex(markerIDS);
	if (robIndex > -1)
	{
		//cout << "Robot Index: " << robIndex << endl;;//returns an index
		putText(frame, intToString(markerThetas[robIndex]) + " deg", Point((int)markerCenters[robIndex].x, (int)markerCenters[robIndex].y + 20), 1, 1, Scalar(0, 255, 0), 2);
		putText(frame, intToString((int)markerCenters[robIndex].x) + "," + intToString((int)markerCenters[robIndex].y), Point((int)markerCenters[robIndex].x, (int)markerCenters[robIndex].y + 35), 1, 1, Scalar(0, 255, 0), 1);

		/*
		if (markerIDS[2] == ROBOT_MARKER_ID)
		{

		putText(frame, intToString(markerThetas[ROBOT_MARKER_ID]), Point((int)markerCenters[ROBOT_MARKER_ID].x, (int)markerCenters[ROBOT_MARKER_ID].y + 20), 1, 1, Scalar(0, 255, 0), 2);
		cout << "Robot angle : " << intToString(markerThetas[ROBOT_MARKER_ID]) << endl;
		}
		*/
	}
	else
	{
		putText(frame, "Robot not found", Point(0, 50), 1, 2, Scalar(0, 255, 0), 2);
		//cout << "Robot not found" << endl;
	}
}

void selectToTrack()
{
	//some boolean variables for different functionality within this
	//program
	bool trackObjects = true;
	bool useMorphOps = true;
	calibrationMode = true;
	//Matrix to store each frame of the webcam feed
	Mat cameraFeed;
	//matrix storage for HSV image
	Mat HSV;
	//matrix storage for binary threshold image
	Mat threshold;
	//x and y values for the location of the object
	//int x = 0, y = 0;       // we are now making this a Global variable

	//video capture object to acquire webcam feed
	VideoCapture capture;
	//open capture object at location zero (default location for webcam)
	capture.open(1);
	//set height and width of capture frame
	capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);
	//must create a window before setting mouse callback
	cv::namedWindow(windowName);
	//set mouse callback function to be active on "Webcam Feed" window
	//we pass the handle to our "frame" matrix so that we can draw a rectangle to it
	//as the user clicks and drags the mouse
	cv::setMouseCallback(windowName, clickAndDrag_Rectangle, &cameraFeed);
	//initiate mouse move and drag to false 
	mouseIsDragging = false;
	mouseMove = false;
	rectangleSelected = false;

	//start an infinite loop where webcam feed is copied to cameraFeed matrix
	//all of our operations will be performed within this loop
	while (1) {
		//store image to matrix
		capture.read(cameraFeed);
		//convert frame from BGR to HSV colorspace
		cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
		//set HSV values from user selected region
		recordHSV_Values(cameraFeed, HSV);
		//filter HSV image between values and store filtered image to
		//threshold matrix
		inRange(HSV, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), threshold);
		//perform morphological operations on thresholded image to eliminate noise
		//and emphasize the filtered object(s)
		if (useMorphOps)
			morphOps(threshold);
		//pass in thresholded frame to our object tracking function
		//this function will return the x and y coordinates of the
		//filtered object
		if (trackObjects)
			trackFilteredObject(x, y, threshold, cameraFeed);

		//show frames 
		if (calibrationMode == true) {

			//create slider bars for HSV filtering
			createTrackbars();
			imshow(windowName1, HSV);
			imshow(windowName2, threshold);
		}
		else {

			destroyWindow(windowName1);
			destroyWindow(windowName2);
			destroyWindow(trackbarWindowName);
		}
		imshow(windowName, cameraFeed);



		//delay 30ms so that screen can refresh.
		//image will not appear without this waitKey() command
		//also use waitKey command to capture keyboard input
		if (waitKey(30) == 99) calibrationMode = !calibrationMode;//if user presses 'c', toggle calibration mode
		else if (waitKey(30) >= 0) break;
	}

}

int startDetection_Track(const Mat& cameraMatrix, const Mat& distanceCoefficients, float arucoSquareDimensions)
{



	//some boolean variables for different functionality within this
	//program
	bool trackObjects = true;
	bool useMorphOps = true;
	calibrationMode = true;

	//Mat frame; made Global
	Mat res, cameraFeed; //Captured frame in Original: Frame, Undistorted: res (result)

						 //matrix storage for HSV image
	Mat HSV;
	//matrix storage for binary threshold image
	Mat threshold;
	//x and y values for the location of the object
	//int x = 0, y = 0;   //we made this a Global variable coordinate for the object/ball

	//**************************
	vector<int> markerIds;
	vector<vector<Point2f>> markerCorners, rejectedCandidates;
	aruco::DetectorParameters paremeters;
	Ptr < aruco::Dictionary> markerDictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50);
	namedWindow("Webcam", CV_WINDOW_AUTOSIZE);
	namedWindow(windowName);

	//****************************

	VideoCapture vid(1);

	//open capture object at location zero (default location for webcam)
	vid.open(1);
	//set height and width of capture frame
	vid.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	vid.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

	//as the user clicks and drags the mouse
	cv::setMouseCallback(windowName, clickAndDrag_Rectangle, &cameraFeed);

	//initiate mouse move and drag to false 
	mouseIsDragging = false;
	mouseMove = false;
	rectangleSelected = false;

	//***********************************


	if (!vid.isOpened())
	{
		return -1;
	}

	vector<Vec3d> rotationVectors, translationVectors;



	while (true)
	{
		//HSV selection and object detection***************************

		//store image to matrix
		vid.read(cameraFeed);
		//convert frame from RGB to HSV colorspace
		cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
		//set HSV values from user selected region
		recordHSV_Values(cameraFeed, HSV);
		//filter HSV image between values and store filtered image to
		//threshold matrix
		inRange(HSV, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), threshold);
		//perform morphological operations on thresholded image to eliminate noise
		//and emphasize the filtered object(s)
		if (useMorphOps)
			morphOps(threshold);
		//pass in thresholded frame to our object tracking function
		//this function will return the x and y coordinates of the
		//filtered object
		if (trackObjects)
			trackFilteredObject(x, y, threshold, cameraFeed);

		//show frames 
		if (calibrationMode == true) {

			//create slider bars for HSV filtering
			//createTrackbars();
			//imshow(windowName1, HSV);
			imshow(windowName2, threshold);
		}
		else {

			destroyWindow(windowName1);
			destroyWindow(windowName2);
			destroyWindow(trackbarWindowName);
		}



		//Aruco markers stuff******************************************

		if (!vid.read(frame))
			continue;
		//break;
		undistort(frame, res, cameraMatrix, distanceCoefficients);
		aruco::detectMarkers(frame, markerDictionary, markerCorners, markerIds);
		//aruco::detectMarkers(res, markerDictionary, markerCorners, markerIds); //undistorted
		aruco::estimatePoseSingleMarkers(markerCorners, arucoSquareDimension, cameraMatrix, distanceCoefficients, rotationVectors, translationVectors);

		//calculate MarkerCenter and Angle
		calculateMarkerCenterAndAngle(markerIds, markerCorners, markerCenters, markerThetas);
		//display Robot Angle close to Aruco Marker
		/* //display all angles of every marker
		for (int i = 0; i < markerIds.size(); i++)
		{
		putText(frame, intToString(markerThetas[i]), Point((int)markerCenters[i].x, (int)markerCenters[i].y + 20), 1, 1, Scalar(0, 255, 0), 2);
		}
		*/
		//display just the robots

		calculateRobotMarkerCenterAndAngle(frame, markerIds); //distorted
															  //calculateRobotMarkerCenterAndAngle(res, markerIds); //undistorted

															  //draw markers////////////////////////////////////
		if (markerIds.size() > 0)
		{
			aruco::drawDetectedMarkers(frame, markerCorners, markerIds);
			//aruco::drawDetectedMarkers(res, markerCorners, markerIds);   //aruco marker detection on undistorted capture
			//cout << "Found IDs:" << endl;
			//cout << Mat(markerIds) << endl;
			//aruco::drawMarker(markerDictionary, markerIds[0], 100, frame, 1); //prints detected aruco marker
		}
		else if (rejectedCandidates.size() > 0)
		{
			aruco::drawDetectedMarkers(frame, rejectedCandidates, noArray(), Scalar(100, 0, 255));
			//aruco::drawDetectedMarkers(res, rejectedCandidates, noArray(), Scalar(100, 0, 255));    //aruco marker detection on undistorted capture
		}

		/*
		//draw axis//////////////////////////////////////
		for (int i = 0; i < markerIds.size(); i++)
		{
		aruco::drawAxis(frame, cameraMatrix, distanceCoefficients, rotationVectors[i], translationVectors[i], 0.09f);
		//aruco::drawAxis(res, cameraMatrix, distanceCoefficients, rotationVectors[i], translationVectors[i], 0.09f); //aruco marker draw axis on undistorted capture
		}
		*/


		//draw rectangle between two points (markerId 0 & 1)
		vector<Point2f> fieldCorners;
		bool corner1Found = false, corner2Found = false;

		if (markerIds.size() > 1)
		{
			//need to designate marker ID 1 to markerCorner[0][0]
			//int width = abs((int)markerCorners[1][0].x - (int)markerCorners[0][0].x);
			//cout << "Width" << width << endl;
			//int height = abs((int)markerCorners[1][0].y - (int)markerCorners[0][0].y);
			//cout << "Height" << height << endl;

			//cv::Rect rect((int)markerCorners[0][0].x, (int)markerCorners[0][0].y, width, height);
			//cv::Point pt1((int)markerCorners[0][0].x, (int)markerCorners[0][0].y);
			//cv::Point pt2((int)markerCorners[1][0].x, (int)markerCorners[1][0].y);

			//Robert inspired code
			for (int i = 0; i < markerIds.size(); i++)
			{
				if (markerIds[i] == CORNER_MARKER_ID_0)
				{
					corner1Found = true;
					fieldCorners.push_back(markerCorners[i][0]);
					continue;
				}
				if (markerIds[i] == CORNER_MARKER_ID_1)
				{
					corner2Found = true;
					fieldCorners.push_back(markerCorners[i][0]);
					continue;
				}
			}
			if (corner1Found && corner2Found)
			{
				cv::rectangle(frame, fieldCorners[0], fieldCorners[1], cv::Scalar(0, 255, 0)); //create rect on regular capture
																							   //cv::rectangle(res, fieldCorners[0], fieldCorners[1], cv::Scalar(0, 255, 0)); //create rect on undistorted capture 
			}
		}

		/*
		//Prints out the coordinate of the corner origin of the aruco marker
		for (int a = 0; a < markerIds.size(); a++)
		{
		cout << "marker : " << endl;
		cout << markerIds[a] << endl;
		//cout << (int)markerCorners[a][0].x << endl; //to separate the coordinates
		//cout << (int)markerCorners[a][0].y << endl;
		//cout << "markerCenter : " << (int)markerCenter.x << ", " << (int)markerCenter.y << endl;
		//cout << "Robot Theta: " << RobotTheta << endl;
		}
		*/

		//Prints out and finds the middle of the two aruco markers, making it the new origin of the coordinate


		imshow("Webcam", frame);
		imshow(windowName, cameraFeed);

		//imshow("Undistorted", res);
		//cout << "END " << endl;
		//cout << endl;
		if (waitKey(30) >= 0) break;
	}

	return 1;
}

int commandAngle(int targetTheta)
{
	//note:
	//when Robot is has 0 degree angle:
	//  - turning left makes theta go towards 180 deg. 
	//  - turning right makes theta go towards -180 deg
	while (abs(abs(targetTheta) - abs(RobotTheta)) > 3) //threshold for angle
	{
		if (targetTheta < RobotTheta)
		{
			cout << "Robot is turning right" << endl;
		}
		else//(targetTheta > RobotTheta) 
		{
			cout << "Robot is turning left" << endl;
		}
		//return -1;
	}
	cout << "Robot Theta is as instructed" << endl;
	return 1;
}

int commandLocation(Point2f targetLoc)
{
	while (targetLoc != markerCenter)
	{
		putText(frame, intToString((int)targetLoc.x) + "," + intToString((int)targetLoc.y), Point(0, 20), 1, 2, Scalar(0, 255, 0), 2);
		//cout << "x distance away: " << abs(abs((int)targetLoc.x) - abs((int)markerCenter.x)) << endl;
		//cout << "y distance away: " << abs(abs((int)targetLoc.y) - abs((int)markerCenter.y)) << endl;
		cout << "Robot Running Forward" << endl;
	}
	cout << "Robot reached target location" << endl;
	return 1;
}

int main(int argv, char** argc)
{
	Mat cameraMatrix = Mat::eye(3, 3, CV_64F);
	Mat distanceCoefficients;

	// Aruco Marker ***********
	//cameraCalibrationProcess(cameraMatrix,distanceCoefficients);    //i. Use this first to calibrate the Camera/Lense

	loadCameraCalibration("IloveCameraCalibration", cameraMatrix, distanceCoefficients);      //ii. loading laptop's camera calibration
																							  //loadCameraCalibration("IloveCameraCalibration_A", cameraMatrix, distanceCoefficients);   //ii. loading senior project webcam parameters

																							  //startWebcamMonitoring(cameraMatrix,distanceCoefficients,arucoSquareDimension);             //iii. start monitoring / detecting

																							  //startDetection_Track(cameraMatrix, distanceCoefficients, arucoSquareDimension);          //iii. combined selectToTrack() and startWebcamMonitoring(...)

																							  //Using Threading
																							  //std::thread t1((startDetection_Track),cameraMatrix, distanceCoefficients, arucoSquareDimension);
	std::thread t2((TCP_IP)); //argument 1 is servername, argument 2 is server ip address

							  //main
	try
	{
		int theta;
		Point2f targetLoc;


		cout << "From main: " << endl;
		cout << "Enter desired angle: ";
		cin >> theta;
		cout << "Enter desired x location: ";
		cin >> targetLoc.x;
		cout << "Enter desired y location: ";
		cin >> targetLoc.y;


		while (RobotIndex > -1)
		{
			commandAngle(theta);
			cout << "out of command angle" << endl;

			commandLocation(targetLoc);
			cout << "out of command location" << endl;
			break;

		}

	}
	catch (...) {
		//t1.join();
		t2.join();
		throw;
	}
	//t1.join();
	t2.join();

	return 0;
}

