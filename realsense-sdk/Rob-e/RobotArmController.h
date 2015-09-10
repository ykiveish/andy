#pragma once
#include "IRobotController.h"

struct CoordinatesDeltas {
	float axisDelta = 0.03;
	float pinchDelta = 40;
	float pitchDelta = 15;  //degrees, not implemented yet, palm up/down angle = pitch
};

class RobotArmController : public IRobotController
{
private:
	RobotBoundaries _robotBoundaries;
	CoordinatesDeltas _coordsDeltas;
	RobotCoordinates _lastCameraCoords;
	bool _isRobotBoundsAccuired;

	bool didCoordsChangeByDelta(const RobotCoordinates coords);

public:
	RobotArmController(INetworkClient* networkClient);
	~RobotArmController();

	bool ConvertCoordinatesToRobot(const RobotCoordinates cameraCoords, RobotCoordinates& outRobotCoords);
	void UpdateCoordinates(const RobotCoordinates coords);
};

