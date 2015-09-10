#pragma once
class RobotBoundaries
{
public:
	float minX, minY, minZ;
	float maxX, maxY, maxZ;
	int minPinch, maxPinch;
	float minPalmPitch, maxPalmPitch;

	RobotBoundaries();

	RobotBoundaries(float minX, float minY, float minZ,
					float maxX, float maxY, float maxZ,
					int minPinch, int maxPinch,
					float minPalmPitch, float maxPalmPitch);
	
	void printRobotBoundaries();

	~RobotBoundaries();
};

