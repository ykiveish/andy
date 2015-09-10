#include "OpenCVCameraClient.h"
#include <iostream>

using namespace std;

OpenCVCameraClient::OpenCVCameraClient(RobotCoordinates* pWebcamRobotCoords, mutex* webCamRobotCoordsMutex)
{
	_pWebcamRobotCoords = pWebcamRobotCoords;
	_webCamRobotCoordsMutex = webCamRobotCoordsMutex;
}

OpenCVCameraClient::~OpenCVCameraClient()
{
	// the camera will be deinitialized automatically in VideoCapture destructor
}

void OpenCVCameraClient::UpdateCoordinates(RobotCoordinates cameraCoords)
{
	if (_webCamRobotCoordsMutex && _pWebcamRobotCoords)
	{
		_webCamRobotCoordsMutex->lock();
		_pWebcamRobotCoords->UpdateCoordinates(cameraCoords.x, cameraCoords.y, cameraCoords.z, cameraCoords.pinch, cameraCoords.palmPitch);
		_webCamRobotCoordsMutex->unlock();
	}
	
}
