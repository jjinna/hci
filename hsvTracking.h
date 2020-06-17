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

using namespace std;
using namespace cv;

#define SIZE 256
#define WIDTH 400
#define HEIGHT 250

void hsvinit();
void getTrackBar();
void createTrackbars();
void hsv_main();
void morphOps(Mat& thresh);
void on_trackbar(int, void*);
void drawObject(int x, int y);
void trackFilteredObject();

string intToString(int number);

