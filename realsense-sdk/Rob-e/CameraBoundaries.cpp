#include "CameraBoundaries.h"
#include <iostream>

using namespace std;

CameraBoundaries::CameraBoundaries()
{
	this->minX = 0;
	this->minY = 0;
	this->minZ = 0;
	this->maxX = 0;
	this->maxY = 0;
	this->maxZ = 0;
}

CameraBoundaries::CameraBoundaries(float minX, float minY, float minZ,
	float maxX, float maxY, float maxZ)
{
	this->minX = minX;
	this->minY = minY;
	this->minZ = minZ;
	this->maxX = maxX;
	this->maxY = maxY;
	this->maxZ = maxZ;
}

CameraBoundaries::~CameraBoundaries()
{
}

void CameraBoundaries::printCameraBoundaries()
{
	cout << endl << "Camera Boundaries: " << endl;
	cout << "minX: " << minX << endl;
	cout << "maxX: " << maxX << endl;
	cout << "minY: " << minY << endl;
	cout << "maxY: " << maxY << endl;
	cout << "minZ: " << minZ << endl;
	cout << "maxZ: " << maxZ << endl;
	cout << endl;
}
