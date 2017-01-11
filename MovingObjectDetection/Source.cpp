/**
Author: Kaushik Roy
Purpose: Initial purpose was to extract frames from video. Then modifed to detect degraded frame using three consecutive frames info.
Date: 9-1-2016
*/

#include <iostream>
#include <vector>
#include <string>
#include <vector>
#include <queue>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
#include <opencv2\opencv.hpp>

using namespace std;
using namespace cv;

const int maxLengthOfFileName = 6, framestoConsider = 5;
int totalFrameCount = 2500;
int isVideoSource = 0;

const string imageFilePathBase = "D:\\dataset2012\\dataset\\cameraJitter\\boulevard\\input\\";

const string videoFilePathBase = "D:\\Research\\test\\GroundTruth_Clip\\";
const vector<string> videoFilePaths =
{
	"1880\\CEQ0001880_000643369.edit.mpg",
	"1882\\CEQ0001882_000643430.edit.mpg",
	"1895\\CEQ0001895_000643637_1.edit.mpg",
};


std::string getFileName(int frameIndx)
{
	std::string prefix = "", filename = to_string(static_cast<long long>(frameIndx));
	for (int i = 1; i <= (maxLengthOfFileName - filename.size()); i++)
	{
		prefix = "0" + prefix;
	}

	filename = imageFilePathBase + "in" + prefix + filename + ".jpg";
	return filename;
}


int main()
{

	//for (int x = 0; x < videoFilePaths.size(); x++)
	{
						
		try {
			VideoCapture cap;
			
			if (isVideoSource)
			{
				//string sourceVideoFilePath = videoFilePathBase + videoFilePaths[x];
				string sourceVideoFilePath = "D:\\Research\\test\\GroundTruth_Clip\\1882\\CEQ0001882_000643430.edit.mpg";
				
				//open the video file
				cap.open(sourceVideoFilePath); // cap.open(0); to open the camera

				// check if we succeeded
				if (!cap.isOpened())
					CV_Error(CV_StsError, "Can not open Video file");

				vector<int> compression_params;
				compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
				compression_params.push_back(100);

				//cap.get(CV_CAP_PROP_FRAME_COUNT) contains the number of frames in the video;
				totalFrameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
			}
			
			std::cout << "\nTotal Frame Count: " << totalFrameCount;

			queue<Mat> cannyEdges, cannyEdgeDiffs;
			Mat cannyEdgeDiffAcc, cannyEdgeDiffAccGlobal;

			for (int currFrameIndx = 1; currFrameIndx <= totalFrameCount; currFrameIndx++)
			{
				std::cout << currFrameIndx << " ";
				Mat prev_frame, curr_frame, src_gray, detected_edges;

				if(isVideoSource)
					cap >> curr_frame; // get the next frame from video
				else
					curr_frame = imread(getFileName(currFrameIndx), 1);

				// RGB to gray scale conversion
				cvtColor(curr_frame, src_gray, CV_BGR2GRAY);
				// Blur
				blur(src_gray, src_gray, cv::Size(3, 3));

				int kernel_size = 3;
				int lowThreshold = 35;
				int highThreshold = 100;

				// Canny Edge Detection
				Canny(src_gray, detected_edges, lowThreshold, highThreshold, kernel_size);

				imshow("Canny Edge", detected_edges);

				detected_edges = detected_edges*(1.0f / 255);

				cannyEdges.push(detected_edges);

				if (currFrameIndx > framestoConsider)
				{
					Mat cannyEdgeDiff;

					if (currFrameIndx == framestoConsider + 1)
					{
						for (int c = 0; c < framestoConsider; c++)
						{
							bitwise_xor(cannyEdges.front(), detected_edges, cannyEdgeDiff);
							
							cannyEdges.pop();
							cannyEdgeDiffs.push(cannyEdgeDiff);

							if (c == 0) {
								cannyEdgeDiffAcc = cannyEdgeDiff;
							}
							else {
								cannyEdgeDiffAcc += cannyEdgeDiff;
							}
						}
						cannyEdgeDiffAcc.copyTo(cannyEdgeDiffAccGlobal);
					}
					else
					{
						Mat oldest_cannyEdgesDiff = cannyEdgeDiffs.front();
						bitwise_xor(cannyEdges.front(), detected_edges, cannyEdgeDiff);

						cannyEdges.pop();

						cannyEdgeDiffs.pop();
						cannyEdgeDiffs.push(cannyEdgeDiff);

						cannyEdgeDiffAcc += cannyEdgeDiff;
						cannyEdgeDiffAcc -= oldest_cannyEdgesDiff;

						cannyEdgeDiffAccGlobal += cannyEdgeDiff;
					}
				}
			}
		}
		catch (cv::Exception& e) {
			cerr << e.msg << endl;
			//getchar();
			//exit(1);
		}

	}

	getchar();
	return 0;
}