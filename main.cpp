#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#pragma comment( lib, "ws2_32.lib")

#include"detecting.h"
#include"hsvTracking.h"

//#define onlyObject

Mat capFrame;
int main() {
	VideoCapture cap(0);
	Mat backgroundSub_binary, backgroundSub_color;
	Mat backgroundSub_removeSkin;
	Mat backgroundSub_bgr;
	WSADATA wsdata;

	extern Ptr<BackgroundSubtractor>pMOG2;
	extern Point center_of_object;

	//createBackgroundSubtractorMOG2
	pMOG2 =cv::createBackgroundSubtractorMOG2(500, 16, false);

	//face_classifier.load("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_frontalface_default.xml");

	//for hsv tracking
	hsvinit();
	
	//for socket
	extern string toSocket;
	int iRes = ::WSAStartup(MAKEWORD(0x02, 0x02), &wsdata);
	if (ERROR_SUCCESS != iRes)
		return 0;

	// 소켓 만들기
	SOCKET hSocket;
	hSocket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == hSocket)
		return 0;

	// 서버에 연결
	SOCKADDR_IN servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servAddr.sin_port = htons(5055);
	iRes = ::connect(hSocket, (LPSOCKADDR)&servAddr, sizeof(servAddr));

	while (true) {
		//1. 웹캠으로 이미지 불러와 사이즈 설정, 좌우반전
		cap.read(capFrame);
		resize(capFrame, capFrame, Size(WIDTH, HEIGHT));
		flip(capFrame, capFrame, 1);
		//imshow("원본", capFrame);
#ifdef onlyObject

		OnlyObject(capFrame);
		::send(hSocket, toSocket.c_str(), toSocket.length(), 0);

#else
		//2. 배경제거
		backgroundSub_binary = subBackground(capFrame, backgroundSub_binary);
		backgroundSub_binary = ~backgroundSub_binary;
		//imshow("backgroundSub_binary", backgroundSub_binary);

		//3. 2 결과 이미지 피부색 제거
		backgroundSub_removeSkin = removeSkinArea(backgroundSub_binary, capFrame);
		//imshow("backgroundSub_removeSkin", backgroundSub_removeSkin);

		//4. 3 결과 이미지 gray->bgr
		cvtColor(backgroundSub_removeSkin, backgroundSub_bgr, COLOR_GRAY2BGR);

		//5. 4 결과 이미지 edge detection + labeling
		Mat edgeDetectVideo = edgeDetect(backgroundSub_removeSkin);

		backgroundSub_color = capFrame + backgroundSub_bgr;
		//imshow("temp", backgroundSub_color);
		
		//원본 이미지에서 hsv기준으로 object tracking
		hsv_main();

		//소켓으로 데이터 전송
		::send(hSocket, toSocket.c_str(), toSocket.length(), 0);

#endif
		if (waitKey(27) == 0)	break;
	}
	::closesocket(hSocket);
	WSACleanup();
	waitKey();
}