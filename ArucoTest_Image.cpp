// ArucoTest_Image.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define INPUT_IMAGE_FILE "ArucoMarkerTest.jpg"

using namespace std;
using namespace cv;
using namespace aruco;


int main()
{
	Mat inputImage, outputImage;
	inputImage = imread(INPUT_IMAGE_FILE);//attempt to read an image
	if (inputImage.empty())
	{
		cout << "Could not open: " << INPUT_IMAGE_FILE << endl;
		return -1;//exit if image could not be opened
	}
	resize(inputImage, inputImage, Size(inputImage.cols / 4, inputImage.rows / 4));//resize image to make it more workable
	
	//markerCorners == list of corners of detected objects, 4 corners returned per object
	//markerIDs == list of IDs of each detected marker in markerCorners, markercorners and markerIDs vectors have same size
	//rejectedCandidates == list of marker candidates whose squares were found but do not have expected codes
	vector<int> markerIDs;
	vector<vector<Point2f>> markerCorners, rejectedCandidates;
	//parameters for detection can be customized as needed
	Ptr<DetectorParameters> parameters = DetectorParameters::create();
	Ptr<Dictionary>  dictionary = getPredefinedDictionary(DICT_4X4_50);
	detectMarkers(inputImage, dictionary, markerCorners, markerIDs, parameters, rejectedCandidates);

	inputImage.copyTo(outputImage);//create copy image to mark up
	if (markerIDs.size() > 0)
	{
		drawDetectedMarkers(outputImage, markerCorners, markerIDs);
		cout << "Found IDs:" << endl;
		cout << Mat(markerIDs) << endl;
	}
	else if (rejectedCandidates.size() > 0)
	{
		drawDetectedMarkers(outputImage, rejectedCandidates, noArray(), Scalar(100, 0, 255));
	}
	

	//show original image
	
	namedWindow("Original Image", WINDOW_AUTOSIZE);
	imshow("Original Image", inputImage);
	//show marked up image
	namedWindow("Detected Objects", WINDOW_AUTOSIZE);
	imshow("Detected Objects", outputImage);
	waitKey(0);

    return 0;
}

