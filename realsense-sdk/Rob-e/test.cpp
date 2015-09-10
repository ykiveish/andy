#include "CoordinatesProducerRS.h"
#include "RobotArmController.h"
#include "RobotArmNetworkClient.h"
#include "OpenCVCameraClient.h"
#include <iostream>
#include "opencv2/opencv.hpp"
#include <thread>

using namespace cv;
using namespace std;

void main()
{
	string serverAddress = "192.168.0.101"; //"127.0.0.1"; //"192.168.0.102";  //"192.168.43.73"; 
	string serverPort = "8080";  //"3000"; 
	VideoCapture* cap = NULL;
	RobotCoordinates* pWebcamRobotCoords = new RobotCoordinates(0,0,0,0,0);
	mutex* webCamRobotCoordsMutex = new mutex();
	Mat frame;

	CoordinatesProducerRS coordinatesProducerRS;
	RobotArmNetworkClient* robotArmNetworkClient = new RobotArmNetworkClient(serverAddress, serverPort);
	RobotArmController* andy = new RobotArmController(robotArmNetworkClient);
	OpenCVCameraClient* cameraClient = new OpenCVCameraClient(pWebcamRobotCoords, webCamRobotCoordsMutex);

	coordinatesProducerRS.AddListener(andy);
	coordinatesProducerRS.AddListener(cameraClient);

	std::thread t1(&CoordinatesProducerRS::ProduceCoordinates, coordinatesProducerRS);
	
	cap = new VideoCapture(3); // open the default camera
	if (!cap->isOpened())  // check if we succeeded
		printf("ERROR\n");
	namedWindow("Andy", CV_WINDOW_NORMAL);
	while (true)
	{
		cap->read(frame); // get a new frame from camera
		if (pWebcamRobotCoords != NULL)
		{
			webCamRobotCoordsMutex->lock();
			//printf("Drawing circle with center (X:%f, Y:%f) and Size Z:%f)\n", pWebcamRobotCoords->x, pWebcamRobotCoords->y, pWebcamRobotCoords->z);
			circle(frame, Point( ((0.3 - pWebcamRobotCoords->x) / 0.6) * frame.size().width, ((0.25 - pWebcamRobotCoords->y) / 0.5) * frame.size().height ), pWebcamRobotCoords->z * 50, Scalar(0, 0, 255 * pWebcamRobotCoords->pinch / 100), CV_FILLED, 8);
			webCamRobotCoordsMutex->unlock();
		}
		imshow("Andy", frame);
		if (cv::waitKey(30) >= 0) break;
	}

	//never called - TODO - some keypress to end application
	cout << "main finished\n";
	delete(webCamRobotCoordsMutex); 
	delete(andy);
	delete(robotArmNetworkClient);
	delete(cameraClient);
	delete(pWebcamRobotCoords);


}
