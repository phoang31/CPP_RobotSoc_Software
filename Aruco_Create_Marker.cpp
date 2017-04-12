// Aruco_Create_Marker.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
using namespace std;
using namespace cv;
using namespace aruco;

//generates a set of aruco markers to print out
void createArucoMarkers()
{
	Mat markerImage;
	cout << "Generating 4x4_50 Aruco markers, indexes 0-49. Size: 300x300px." << endl;
	String baseFileStr = "ArucoMarker_4x4_50_300x300px_ID#";
	String imageFileStr;
	//marker IDs allowed from 0 to 49, can go higher using different dict
	Ptr<Dictionary> dictionary = getPredefinedDictionary(DICT_4X4_50);
	
	for (int i = 0; i < 50; i++)
	{
		drawMarker(dictionary, i, 300, markerImage);//can set square size and border thickness

		imageFileStr = baseFileStr + to_string(i) + ".png";
		imwrite(imageFileStr, markerImage);
	}
	cout << "Done." << endl;
}

int main()
{
	createArucoMarkers();
    return 0;
}

