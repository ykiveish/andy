#pragma once
#include "ICoordinatesListener.h"
#include <mutex>

class OpenCVCameraClient : public ICoordinatesListener
{
private:
	RobotCoordinates* _pWebcamRobotCoords;
	std::mutex* _webCamRobotCoordsMutex;

public:
	OpenCVCameraClient(RobotCoordinates* pWebcamRobotCoords, std::mutex* webCamRobotCoordsMutex);
	~OpenCVCameraClient();

	void UpdateCoordinates(RobotCoordinates coords);
};