#pragma once

#include "INetworkClient.h"
#include "RobotCoordinates.h"
#include "RobotBoundaries.h"

class RobotArmNetworkClient : public INetworkClient
{
private:
	std::string _serverAddress;
	std::string _serverPort;

	bool sendHttpRestCall(const std::string serverAddress, const std::string serverPort,
						  const std::string apiCommand, std::string& outResponseStr);
	bool parseJsonToRobotBounderies(const std::string inJsonStr, RobotBoundaries& outRobotBounds);
	bool convertRobotCoordsToUrlString(RobotCoordinates coordinates, std::string& outCoordsAsUrlStr);
	
public:
	RobotArmNetworkClient(const std::string serverAddress, const std::string serverPort);
	~RobotArmNetworkClient();

	bool updateServerAddresss(const std::string newServerAddress);
	bool updateServerPort(const std::string newServerPort);
	bool SendRobotCoordinates(const RobotCoordinates coordinates);
	bool GetRobotBoundaries(RobotBoundaries& outRobotBaoundaries);
	bool RobotConnect();	 //for persistant connetions only
	bool RobotDisconnect(); //for persistant connetions only
};
