#include"hsvTracking.h"


//trackbar에 표시될 string(trackbar 이름)
string windowName = "Original Image";
string windowName1 = "HSV Image";
string windowName2 = "Thresholded Image";
string windowName3 = "After Morphological Operations";
string trackbarWindowName = "Trackbars";

//trackbar에 표시될 값
//HSV값
int H_MIN = 0, H_MAX = 256;
int S_MIN = 0, S_MAX = 256;
int V_MIN = 0, V_MAX = 256;

//trackbar에 표시될 값
//RGB값
int R = 0, G = 0, B = 0;

//trackbar에 표시될 값
//switching
int switching = 0;

//특정 위치에서의 물체 hsv값
int h, s, v;

Mat cameraFeed, thresHold, hsv;
extern Mat capFrame;
string toSocket = "";

const int MAX_NUM_OBJECTS = 30;	//최대 물체 개수
const int MIN_OBJECT_AREA = 40 * 40;	//최소 물체 영역(크기)
const int MAX_OBJECT_AREA = HEIGHT * WIDTH / 1.5;	//최대 물체 영역(크기)

Mat showRGBValue;
extern Point center_of_object;

//빈 함수
void on_trackbar(int, void*){}


//Trackbar 생성 및 초기화
void createTrackbars() {
	//window 설정
	namedWindow(trackbarWindowName, 1);
	cv::resizeWindow(trackbarWindowName, 500, 800);

	createTrackbar("H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar);
	createTrackbar("H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar);
	createTrackbar("S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar);
	createTrackbar("S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar);
	createTrackbar("V_MIN", trackbarWindowName, &V_MIN, V_MAX, on_trackbar);
	createTrackbar("V_MAX", trackbarWindowName, &V_MAX, V_MAX, on_trackbar);

	createTrackbar("SETTING", trackbarWindowName, &switching, 1, on_trackbar);
}


//트랙바 재설정
void setTrackBar() {
	//트랙바에서 setting 값이 1로 설정된다면
	if (getTrackbarPos("SETTING", trackbarWindowName) == 1) {
		setTrackbarPos("SETTING", trackbarWindowName, 0);

		//setTrackbarPos("R", trackbarWindowName, R);
		//setTrackbarPos("G", trackbarWindowName, G);
		//setTrackbarPos("B", trackbarWindowName, B);

		showRGBValue = Mat(350, 500, CV_8UC3, Scalar(B, G, R));
		imshow(trackbarWindowName, showRGBValue);

		setTrackbarPos("H_MIN", trackbarWindowName, h - 30);
		setTrackbarPos("H_MAX", trackbarWindowName, h + 30);
		setTrackbarPos("S_MIN", trackbarWindowName, s - 30);
		setTrackbarPos("S_MAX", trackbarWindowName, s + 30);
		setTrackbarPos("V_MIN", trackbarWindowName, v - 15);
		setTrackbarPos("V_MAX", trackbarWindowName, v + 15);
	}

}

void drawObject(int x, int y) {
	circle(capFrame, cv::Point(x, y), 10, cv::Scalar(0, 0, 255));
	putText(capFrame, to_string(x) + " , " + to_string(y), Point(x, y + 20), 1, 1, Scalar(0, 255, 0));

	toSocket = to_string(x - (WIDTH / 2)) + " " + to_string((HEIGHT / 2) - y);
}

void morphOps() {
	Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3, 3));
	Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8,8));

	erode(thresHold, thresHold, erodeElement,Point(-1,-1),3);
	dilate(thresHold, thresHold, dilateElement, Point(-1, -1), 3);
}

void trackFilteredObject() {
	int x, y;

	Mat temp;
	thresHold.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	//use moments method to find our filtered object
	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if (numObjects < MAX_NUM_OBJECTS) {
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
				if (area > MIN_OBJECT_AREA) {
					x = moment.m10 / area;
					y = moment.m01 / area;

					objectFound = true;
				}
				else objectFound = false;


			}
			if (objectFound == true) {
				//draw object location on screen
				drawObject(x, y);
			}

		}
		else putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
	}
}

void hsvinit() {
	showRGBValue = Mat(350, 500, CV_8UC3, Scalar(0, 0, 0));
	createTrackbars();
}

void hsv_main() {
	//convert frame from BGR to HSV colorspace
	cvtColor(capFrame, hsv, COLOR_BGR2HSV);

	if (center_of_object.x != -1 && center_of_object.y != -1 && center_of_object.x > 0 && center_of_object.y > 0) {
		h = hsv.at<cv::Vec3b>(center_of_object.y, center_of_object.x)[0];
		s = hsv.at<cv::Vec3b>(center_of_object.y, center_of_object.x)[1];
		v = hsv.at<cv::Vec3b>(center_of_object.y, center_of_object.x)[2];
		//printf("%d %d %d\n", h, s, v);

		B = capFrame.at<cv::Vec3b>(center_of_object.y, center_of_object.x)[0];
		G = capFrame.at<cv::Vec3b>(center_of_object.y, center_of_object.x)[1];
		R = capFrame.at<cv::Vec3b>(center_of_object.y, center_of_object.x)[2];

	}
	cvtColor(capFrame, hsv, COLOR_BGR2HSV);
	inRange(hsv, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), thresHold);
	morphOps();
	imshow(windowName2, thresHold);
	trackFilteredObject();

	imshow(windowName, capFrame);
	//imshow(windowName1,HSV);

	setTrackBar();

}