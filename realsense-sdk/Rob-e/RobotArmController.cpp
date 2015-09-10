#include <iostream>
#include <math.h>

#include "RobotArmController.h"
#include "RobotBoundaries.h"

using namespace std;

RobotArmController::RobotArmController(INetworkClient* networkClient) : IRobotController(networkClient), _isRobotBoundsAccuired(false)
{
}

RobotArmController::~RobotArmController()
{
}

bool RobotArmController::ConvertCoordinatesToRobot(const RobotCoordinates cameraCoords, RobotCoordinates& outRobotCoords)
{
	outRobotCoords.UpdateCoordinates(0, 0, 0, 0, cameraCoords.palmPitch);//reset
	bool isGetBoundsSuccess = false;

	if (!_isRobotBoundsAccuired) //first time, take boundaries from robot
	{
		isGetBoundsSuccess = _networkClient->GetRobotBoundaries(_robotBoundaries);
		if (!isGetBoundsSuccess)
		{
			printf("RobotArmController::ConvertCoordinatesToRobot - Error, could not get robot boundaries!\n");
			return false;
		}

		_isRobotBoundsAccuired = true;
		_robotBoundaries.printRobotBoundaries();
	}

	outRobotCoords.x = cameraCoords.x * 10 / 0.3;			// * max range / scale 0.3   [-0.3 , 0.3 ]
	outRobotCoords.x = -outRobotCoords.x;					// our left is robot's right
	outRobotCoords.y = (cameraCoords.z - 0.16) * 10 / 0.79; // * max range / scale 0.79  [0.16 , 0.95]
	outRobotCoords.y = 10 - outRobotCoords.y;				// our in is robot's out
	outRobotCoords.z = (cameraCoords.y + 0.25) * 10 / 0.5;  // * max range / scale 0.25  [-0.25 , 0.25]
	if (cameraCoords.pinch < 20)
	{
		outRobotCoords.pinch = 0;
	}
	else if (cameraCoords.pinch > 80)
	{
		outRobotCoords.pinch = 1;
	}
	printf("\nRobot Coords: X: %f ,Y: %f ,Z: %f ,Pinch: %d, palmPitch: %f\n", outRobotCoords.x, outRobotCoords.y, outRobotCoords.z, outRobotCoords.pinch, outRobotCoords.palmPitch);

	return true;
}

void RobotArmController::UpdateCoordinates(const RobotCoordinates cameraCoords)
{

	bool isSuccess = false;
	bool isNeedToSendCoordinates = true;
	RobotCoordinates robotCoords(0, 0, 0, 0, 0);

	//printf("\n\nCamera Coords: X: %f ,Y: %f ,Z: %f ,Pinch: %d, palmPitch: %f\n", cameraCoords.x, cameraCoords.y, cameraCoords.z, cameraCoords.pinch, cameraCoords.palmPitch);
	//printf("Last Coords: X: %f ,Y: %f ,Z: %f ,Pinch: %d, palmPitch: %f\n\n", _lastCameraCoords.x, _lastCameraCoords.y, _lastCameraCoords.z, _lastCameraCoords.pinch, _lastCameraCoords.palmPitch);

	isNeedToSendCoordinates = didCoordsChangeByDelta(cameraCoords);
	if (isNeedToSendCoordinates)
	{
		isSuccess = ConvertCoordinatesToRobot(cameraCoords, robotCoords);
		if (!isSuccess)
		{
			printf("RobotArmController::UpdateCoordinates - Error, could not convert camera coordinates to robot coords!\n");
			return;
		}

		isSuccess = _networkClient->SendRobotCoordinates(robotCoords);
		if (!isSuccess)
		{
			printf("RobotArmController::UpdateCoordinates - Error, could not send coordinates to robot!\n");
			return;
		}

		_lastCameraCoords.UpdateCoordinates(cameraCoords.x, cameraCoords.y, cameraCoords.z, cameraCoords.pinch, cameraCoords.palmPitch);

		printf("RobotArmController::UpdateCoordinates - coordinates SENT to robot\n");
	}
}

bool RobotArmController::didCoordsChangeByDelta(const RobotCoordinates coords)
{
	float deltaRadius = sqrtf( pow( (coords.x - _lastCameraCoords.x), 2.0 ) +
							   pow( (coords.y - _lastCameraCoords.y), 2.0 ) +
							   pow( (coords.z - _lastCameraCoords.z), 2.0 )
							 );
	int deltaPinch = abs(coords.pinch - _lastCameraCoords.pinch);
	float deltaPitch = abs(coords.palmPitch - _lastCameraCoords.palmPitch); //not yet implemented 
	//cout << "RobotArmController:didCoordsChangeByDelta: deltaRadius=" << deltaRadius << " , deltaPinch=" << deltaPinch << endl;

	if (deltaRadius > _coordsDeltas.axisDelta || deltaPinch > _coordsDeltas.pinchDelta || deltaPitch >= _coordsDeltas.pitchDelta )
	{
		//cout << "RobotArmController:didCoordsChangeByDelta: CHANGED BY DELTA"<< endl;
		return true;
	}
	return false;
}


