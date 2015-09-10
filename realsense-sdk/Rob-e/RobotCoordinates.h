#pragma once
class RobotCoordinates
{
public:
	float x, y, z;
	int pinch;
	float palmPitch;
	
	RobotCoordinates();

	RobotCoordinates(float x, float y, float z, int pinch, float palmPitch);
	~RobotCoordinates();
	void UpdateCoordinates(float x, float y, float z, int pinch, float palmPitch);

};