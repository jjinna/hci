#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include<string>
#include<iostream>
#include<vector>
#include <sstream>
#include <string>
#include <iostream>
#include <opencv\highgui.h>
#include <opencv2/opencv.hpp>
#include<cstdio>

using namespace cv;
using namespace std;

#define SIZE 256
#define WIDTH 400
#define HEIGHT 250

Mat edgeDetect(Mat src);
Mat find_Contours(Mat src_image);
Mat subBackground(Mat src, Mat dst);
Mat removeSkinArea(Mat src_binary, Mat src_bgr);
Mat createImage(Size size, int depth, int nChannels);

void faceDetect(cv::Mat src);

void OnlyObject(cv::Mat src);