#pragma once

#include "INetworkClient.h"
#include "ICoordinatesProducer.h"
#include "ICoordinatesListener.h"
#include "RobotCoordinates.h"
#include "RobotBoundaries.h"

class IRobotController : public ICoordinatesListener
{
protected:
	INetworkClient* _networkClient = NULL;

public:
	IRobotController(INetworkClient* networkClient)
	{
		_networkClient = networkClient;
	}

	virtual bool ConvertCoordinatesToRobot(const RobotCoordinates cameraCoords, RobotCoordinates& outRobotCoords) = 0;
};