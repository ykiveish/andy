#pragma once
#include "RobotCoordinates.h"
#include "RobotBoundaries.h"
#include <string>

class INetworkClient
{
	
public:
	virtual bool SendRobotCoordinates(const RobotCoordinates coordinates) = 0;
	virtual bool RobotConnect() = 0;		
	virtual bool RobotDisconnect() = 0;		
	virtual bool GetRobotBoundaries(RobotBoundaries& outRobotBoundaries) = 0;
};

