#include "RobotBoundaries.h"
#include <iostream>

using namespace std;

RobotBoundaries::RobotBoundaries()
{
	this->minX = 0;
	this->minY = 0;
	this->minZ = 0;
	this->maxX = 0;
	this->maxY = 0;
	this->maxZ = 0;
	this->minPinch = 0;
	this->maxPinch = 0;
	this->minPalmPitch = 0;
	this->maxPalmPitch = 0;
}

RobotBoundaries::RobotBoundaries(float minX, float minY, float minZ,
								float maxX, float maxY, float maxZ, 
								int minPinch, int maxPinch,
								float minPalmPitch, float maxPalmPitch)
{
	this->minX = minX;
	this->minY = minY;
	this->minZ = minZ;
	this->maxX = maxX;
	this->maxY = maxY;
	this->maxZ = maxZ;
	this->minPinch = minPinch;
	this->maxPinch = maxPinch;
	this->minPalmPitch = minPalmPitch;
	this->maxPalmPitch = maxPalmPitch;
}


RobotBoundaries::~RobotBoundaries()
{
}


void RobotBoundaries::printRobotBoundaries()
{
	cout << endl << "Robot Boundaries: " << endl;
	cout << "minX: " << minX << endl;
	cout << "maxX: " << maxX << endl;
	cout << "minY: " << minY << endl;
	cout << "maxY: " << maxY << endl;
	cout << "minZ: " << minZ << endl;
	cout << "maxZ: " << maxZ << endl;
	cout << "minPinch: " << minPinch << endl;
	cout << "maxPinch: " << maxPinch << endl;
	cout << "minPalmPitch: " << minPalmPitch << endl;
	cout << "maxPalmPitch: " << maxPalmPitch << endl;
	cout << endl;
}