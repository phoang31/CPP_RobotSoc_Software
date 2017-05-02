// TrackCircle.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;
using namespace cv;

const int CAMERA_INDEX = 0;
const int FPS = 30;
const int WAITKEY_DELAY_MS = 1000 / FPS;
const String CONTROLS_WINDOW_NAME = "Ball Tracking Controls";
const String THRESH_WINDOW_NAME = "Thresholded";
const String CAMERA_WINDOW_NAME = "Camera Feed";
const int CANNY_UPPER_THRESH_MAX = 255;
const int CENTER_THRESH_MAX = 100;
const int BALL_MIN_RADIUS_MAX = 100;
const int BALL_MAX_RADIUS_MAX = 100;
const int BLUR_KERNEL_RADIUS_MAX = 10;


int main()
{
	Mat frame, gray, result;
	vector<Vec3f> circles;
	deque<Point2f> circleLocationBuffer;
	char keypress = 0;
	int cannyUpperThreshValue = 100;
	int centerDetectionThreshValue = 24;
	int minBallRadiusValue = 9;
	int maxBallRadiusValue = 15;
	int blurKernelRadiusValue = 1;

	namedWindow(CONTROLS_WINDOW_NAME, WINDOW_AUTOSIZE); // Create Window for trackbar
	createTrackbar("Canny Thresh.", CONTROLS_WINDOW_NAME, &cannyUpperThreshValue, CANNY_UPPER_THRESH_MAX);
	createTrackbar("Center Thresh.", CONTROLS_WINDOW_NAME, &centerDetectionThreshValue, CENTER_THRESH_MAX);
	createTrackbar("Ball min. radius", CONTROLS_WINDOW_NAME, &minBallRadiusValue, BALL_MIN_RADIUS_MAX);
	createTrackbar("Ball max. radius", CONTROLS_WINDOW_NAME, &maxBallRadiusValue, BALL_MAX_RADIUS_MAX);
	createTrackbar("Blur kernel radius", CONTROLS_WINDOW_NAME, &blurKernelRadiusValue, BLUR_KERNEL_RADIUS_MAX);

	VideoCapture cap(CAMERA_INDEX);
	if (!cap.isOpened())
	{
		cout << "Failed to open camera " << CAMERA_INDEX << endl;
		return -1;
	}

	while (cap.read(frame))
	{
		cvtColor(frame, gray, COLOR_BGR2GRAY);
		medianBlur(gray, gray, blurKernelRadiusValue*2+1);//apply a blur to the gray frame to help find circles

		HoughCircles(gray, circles, HOUGH_GRADIENT, 1,
			gray.rows / 16,//alter this to detect circles with different distances from eachother
			cannyUpperThreshValue+1, centerDetectionThreshValue+1, minBallRadiusValue, maxBallRadiusValue);//alter last two parameters for min/max ball size

		//use the first circle found
		if (circles.size() > 0)
		{
			Point2f c = { circles[0][0], circles[0][1] };
			circle(gray, Point(c.x, c.y), circles[0][2], Scalar(0, 0, 255), 3, LINE_AA);
			circle(frame, Point(c.x, c.y), 2, Scalar(0, 255, 0), 3, LINE_AA);
			cout << "Ball found @ : " << c << endl;

			
			//remove last element from sequence each time
			//and draw the path of last 10 points
			circleLocationBuffer.push_back(c);
			if (circleLocationBuffer.size() > 15)
			{
				circleLocationBuffer.pop_front();
				for (int i = 0; i < (circleLocationBuffer.size() - 1); i++)//already checked earlier that buffer size is nonzero
				{
					line(frame, circleLocationBuffer[i], circleLocationBuffer[i + 1], Scalar(0, 0, 255), 2);
				}
			}
		}


		imshow(CAMERA_WINDOW_NAME, frame);
		imshow("blurred grayscale", gray);

		keypress = waitKey(WAITKEY_DELAY_MS);
		if (keypress > 0)
		{
			break;
		}
	}
	
    return 0;
}

