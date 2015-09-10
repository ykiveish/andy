#pragma once
class CameraBoundaries
{
public:
	float minX, minY, minZ;
	float maxX, maxY, maxZ;

	CameraBoundaries();

	CameraBoundaries(float minX, float minY, float minZ,
		float maxX, float maxY, float maxZ);
	
	void printCameraBoundaries();

	~CameraBoundaries();
};