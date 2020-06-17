#include "detecting.h"

// face detection configuration
//cv::CascadeClassifier face_classifier;
//cv::CascadeClassifier upperbody_classifier;

Ptr<BackgroundSubtractor>pMOG2;	//MOG2 Background subtractor
Point center_of_object;
int lbX, lbY, trX, trY;

extern string toSocket;

//배경제거
cv::Mat subBackground(cv::Mat src, cv::Mat dst) {
	//1. background sub
	pMOG2->apply(src, dst); //sub

	//2. opening for removing noise
	Mat mask = getStructuringElement(MORPH_RECT, Size(4, 4), Point(1, 1));
	erode(dst, dst, mask, Point(-1, -1), 5);
	dilate(dst, dst, mask, Point(-1, -1), 5);

	return dst;
}

//피부 영역 제거
cv::Mat removeSkinArea(Mat src_binary, Mat src_bgr) {
	Mat YCrCb;
	cvtColor(src_bgr, YCrCb, CV_BGR2YCrCb);

	Mat Skin_Area;
	inRange(YCrCb, Scalar(0, 130, 90), Scalar(255, 168, 130), Skin_Area);

	Mat ret;
	ret = src_binary + Skin_Area;
	erode(ret, ret, Mat(3, 3, CV_8U, Scalar(1)), Point(-1, -1), 3);
	dilate(ret, ret, Mat(3, 3, CV_8U, Scalar(1)), Point(-1, -1), 5);

	return ret;
}

//canny, find_Contours 써서 edge detection 및 labeling
cv::Mat edgeDetect(cv::Mat src) {
	Mat edge = src;
	//edge d
	Mat erodemask = getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1));
	erode(edge, edge, erodemask, Point(-1, -1), 3);

	Mat dilatemask = getStructuringElement(MORPH_RECT, Size(7, 7), Point(1, 1));
	dilate(edge, edge, dilatemask, Point(-1, -1), 4);

	//edge detection using canny edge detection
	Canny(edge, edge, 30, 200);

	edge = find_Contours(edge);
	return edge;
}


cv::Mat createImage(cv::Size size, int depth, int nChannels)
{
	uchar type = ((nChannels - 1) << 3) + depth;
	cv::Mat dst_image = cv::Mat(size, type, cv::Scalar(0));
	return dst_image;
}

//labeling
cv::Mat find_Contours(cv::Mat src_image) {
	//수행 대상은 명암도 영상임.
	if (src_image.channels() != 1) return cv::Mat();

	// 초기화
	cv::Mat tmp_image = src_image.clone();
	cv::Mat dst_image = createImage(src_image.size(), CV_8U, 3);
	cv::cvtColor(src_image, dst_image, cv::COLOR_GRAY2BGR);

	std::vector< std::vector<cv::Point> > contours;


	/* Retrieves outer and optionally inner boundaries of white (non-zero) connected
	components in the black (zero) background */
	cv::findContours(tmp_image, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE, cv::Point(0, 0));


	vector<Moments> mu(contours.size());
	/* Draws contour outlines or filled interiors on the image */
	int max = -1, maxindex = -1;
	for (size_t i = 0; i < contours.size(); i++)
	{
		mu[i] = moments(contours[i], false);
		if (contourArea(contours[i]) > max) {
			maxindex = i;
			max = contourArea(contours[i]);
		}
	}

	vector<Point> mc(contours.size());
	int radius = 50;
	if (maxindex != -1) {
		mc[maxindex] = Point2f(mu[maxindex].m10 / mu[maxindex].m00, mu[maxindex].m01 / mu[maxindex].m00);
		center_of_object = mc[maxindex];
		circle(src_image, center_of_object, (int)(radius + 0.5), Scalar(130, 255, 100), 2);
	}
	else {
		center_of_object = Point(-1, -1);
	}

	imshow("Origin_frame_pals", src_image);

	// 할당한 메모리 해제
	tmp_image.release();

	return dst_image;


	/*
// 수행 대상은 명암도 영상임.
if (src_image.channels() != 1) return cv::Mat();

// 초기화
cv::Mat tmp_image = src_image.clone();
cv::Mat dst_image = createImage(src_image.size(), CV_8U, 3);
cv::cvtColor(src_image, dst_image, cv::COLOR_GRAY2BGR);

std::vector< std::vector<cv::Point> > contours;


// Retrieves outer and optionally inner boundaries of white (non-zero) connected
//components in the black (zero) background
cv::findContours(tmp_image, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE, cv::Point(0, 0));

int bigIndex = -1, big = -1;
for (size_t i = 0; i < contours.size(); i++) {
	if (contourArea(contours[i]) > big) {
		big = contourArea(contours[i]);
		bigIndex = i;
	}
}

//Draws contour outlines or filled interiors on the image
for (size_t i = 0; i < contours.size(); i++)
{

	cv::drawContours(dst_image, contours, (int)i, cv::Scalar(rand() & 255, rand() & 255, rand() & 255), 2, 8);

	//얼굴인식 안된상태
	if(lbX==-1) cv::drawContours(dst_image, contours, (int)i, cv::Scalar(rand() & 255, rand() & 255, rand() & 255), 2, 8);
	else {
		//얼굴 범위 내에 있는 contours
		if ((contours[i][0].x > lbX&& contours[i][0].y < lbY) && (contours[i][0].x<trX && contours[i][0].y>trY)) {}
		else //얼굴 범위 외에 있는 contours
			cv::drawContours(dst_image, contours, (int)i, cv::Scalar(rand() & 255, rand() & 255, rand() & 255), 2, 8);
	}
}

// 할당한 메모리 해제
tmp_image.release();
return dst_image;*/
}

/*
//얼굴검출
void faceDetect(Mat src) {
	Mat frame = src;
	Mat grayframe;

	// convert captured frame to gray scale & equalize
	cv::cvtColor(frame, grayframe, CV_BGR2GRAY);
	cv::equalizeHist(grayframe, grayframe);

	// a vector array to store the face found
	std::vector<cv::Rect> faces;

	face_classifier.detectMultiScale(grayframe, faces, 1.1, 3, 0, Size(60, 60), Size(230, 230));

	// draw the results
	for (int i = 0; i < faces.size(); i++) {
		cv::Point lb(faces[i].x + faces[i].width, faces[i].y + faces[i].height);
		cv::Point tr(faces[i].x, faces[i].y);

		rectangle(grayframe, lb, tr, cv::Scalar(0, 255, 0), 3, 4, 0);

		//얼굴좌표
		lbX = lb.x; lbY = lb.y;
		trX = tr.x; trY = tr.y;
	}

	if (faces.size() == 0) {
		lbX = -1; lbY = -1;
		trX = -1; trY = -1;
	}
	// print the output
	cv::imshow("webcam1", grayframe);
}
*/

void OnlyObject(cv::Mat src) {
	Mat temp;
	pMOG2->apply(src, temp); //sub
	//skinArea sub
	Mat YCrCb;
	cvtColor(src, YCrCb, CV_BGR2YCrCb);
	Mat Skin_Area;
	inRange(YCrCb, Scalar(0, 130, 90), Scalar(255, 168, 130), Skin_Area);
	temp = temp - Skin_Area;
	erode(temp, temp, Mat(3, 3, CV_8U, Scalar(1)), Point(-1, -1), 2);
	dilate(temp, temp, Mat(3, 3, CV_8U, Scalar(1)), Point(-1, -1), 10);

	int sumX = 0;
	int sumY = 0;
	int numXY = 1;
	bool toBig = false;
	for (int x = 0; x < temp.cols; x++) {
		for (int y = 0; y < temp.rows; y++) {
			if (temp.at<uchar>(y, x) > 200) {
				sumX += x;
				sumY += y;
				numXY++;
				if (numXY > 50000) {
					toBig = true;
					break;
				}
			}
		}
	}
	if (!toBig && numXY > 1000) { // 816, 614 왼쪽 위 부터 0,0 이라서 y는 반대로했음
		toSocket=to_string((sumX / numXY) - (temp.cols / 2)) + " " + to_string((temp.rows / 2) - (sumY / numXY));	}
	else {
		toSocket = "";
	}
	circle(src, Point((sumX / numXY), (sumY / numXY)), 10, Scalar(0, 0, 255));

	imshow("result", src);
}