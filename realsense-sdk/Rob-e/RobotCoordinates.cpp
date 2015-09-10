#include "RobotCoordinates.h"


RobotCoordinates::RobotCoordinates(){}

RobotCoordinates::RobotCoordinates(float x = 0, float y = 0, float z = 0, int pinch = 0, float palmPitch = 0)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->pinch = pinch;
	this->palmPitch = palmPitch;
}

RobotCoordinates::~RobotCoordinates()
{
}

void RobotCoordinates::UpdateCoordinates(float x, float y, float z, int pinch, float palmPitch)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->pinch = pinch;
	this->palmPitch = palmPitch;
}
