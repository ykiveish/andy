#pragma once
#include "RobotCoordinates.h"

class ICoordinatesListener
{
public:
	virtual void UpdateCoordinates(RobotCoordinates coords) = 0;
};

