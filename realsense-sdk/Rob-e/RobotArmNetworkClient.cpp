#include "RobotArmNetworkClient.h"

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <cassert>
#include <exception>

using namespace std;
using namespace boost::asio;
using boost::asio::ip::tcp;

//boost::asio::io_service io_service;

const string API_COMMAND_SET_COORDINATES_PREFIX = "/api/setCoordinates";
const string API_COMMAND_GET_BOUNDERIES = "/api/getBoundaries";


RobotArmNetworkClient::RobotArmNetworkClient(const string serverAddress, const string serverPort)
{
	_serverAddress = serverAddress;
	_serverPort = serverPort;
}

RobotArmNetworkClient::~RobotArmNetworkClient()
{
}


//TODO: send the actual coordinates and not the dummy ones
bool RobotArmNetworkClient::SendRobotCoordinates(RobotCoordinates coordinates)
{
	string sereverResponseStr = "";
	string coordinatesAsUrlStr = "";
	bool isResultSuccess = true;
	string apiCommandSetCoords = API_COMMAND_SET_COORDINATES_PREFIX;
	isResultSuccess = convertRobotCoordsToUrlString(coordinates, coordinatesAsUrlStr);
	if (isResultSuccess)
	{
		apiCommandSetCoords += coordinatesAsUrlStr;
		//cout << "SendRobotCoordinates: apiCommandSetCoords: " << apiCommandSetCoords << endl;
		isResultSuccess = sendHttpRestCall(_serverAddress, _serverPort, apiCommandSetCoords, sereverResponseStr);
		//cout << "RobotArmNetworkClient::SendRobotCoordinates recieved serverResponseStr: " << sereverResponseStr << endl;
	}
	return isResultSuccess;
}

bool RobotArmNetworkClient::RobotConnect()
{
	//since this is a REST API, no need to maintain a connection.
	return true;
}

bool RobotArmNetworkClient::RobotDisconnect()
{
	//since this is a REST API, no need to maintain a connection. connection is closed after every api call
	//TODO: think if this is the right way to do this
	return true;
}

//TODO: return the actual bounderies and not the dummy ones
bool RobotArmNetworkClient::GetRobotBoundaries(RobotBoundaries& outRobotBaoundaries)
{
	string serverResponseJsonStr;
	bool isResultSuccess = true;

	isResultSuccess = sendHttpRestCall(_serverAddress, _serverPort, API_COMMAND_GET_BOUNDERIES, serverResponseJsonStr);
	//cout << "RobotArmNetworkClient::GetRobotBoundaries recieved serverResponseJsonStr: " << serverResponseJsonStr << endl;
	if (isResultSuccess)
	{
		isResultSuccess = parseJsonToRobotBounderies(serverResponseJsonStr, outRobotBaoundaries);
		if (isResultSuccess)
		{
			//outRobotBaoundaries.printRobotBoundaries();
			return true;
		}
	}

	return false;
}


bool RobotArmNetworkClient::updateServerAddresss(const std::string newServerAddress)
{
	_serverAddress = newServerAddress;
	return true;
}

bool RobotArmNetworkClient::updateServerPort(const std::string newServerPort)
{
	_serverPort = newServerPort;
	return true;
}

/*
{ "robotBounderies": {"minX": "0", "maxX": "0", "minY": "0", "maxY": "0", "minZ": "0", "maxZ": "0", "minPinch": "0", "maxPinch": "0", minPalmPitch:"0", maxPalmPitch:"0" } }
*/
bool RobotArmNetworkClient::parseJsonToRobotBounderies(const string inJsonStr, RobotBoundaries& outRobotBounds)
{
	try
	{
		stringstream strStream(inJsonStr);

		boost::property_tree::ptree propTree;
		boost::property_tree::read_json(strStream, propTree);
		
		outRobotBounds.minX = propTree.get<float>("robotBoundaries.minX");
		outRobotBounds.minY = propTree.get<float>("robotBoundaries.minY");
		outRobotBounds.minZ = propTree.get<float>("robotBoundaries.minZ");
		outRobotBounds.maxX = propTree.get<float>("robotBoundaries.maxX");
		outRobotBounds.maxY = propTree.get<float>("robotBoundaries.maxY");
		outRobotBounds.maxZ = propTree.get<float>("robotBoundaries.maxZ");
		outRobotBounds.minPinch = propTree.get<int>("robotBoundaries.minPinch");
		outRobotBounds.maxPinch = propTree.get<int>("robotBoundaries.maxPinch");
		outRobotBounds.minPalmPitch = propTree.get<int>("robotBoundaries.minPalmPitch");
		outRobotBounds.maxPalmPitch = propTree.get<int>("robotBoundaries.maxPalmPitch");

		return true;
	}
	catch (std::exception const& e)
	{
		cout << "RobotArmNetworkClient::parseJsonToRobotBounderies - Caught exception: " << e.what() << "\n";
		return false;
	}

}

bool RobotArmNetworkClient::convertRobotCoordsToUrlString(RobotCoordinates coordinates, string& outCoordsAsUrlStr)
{
	outCoordsAsUrlStr = "";
	outCoordsAsUrlStr += "/";
	outCoordsAsUrlStr += to_string(coordinates.x);
	outCoordsAsUrlStr += "/";
	outCoordsAsUrlStr += to_string(coordinates.y);
	outCoordsAsUrlStr += "/";
	outCoordsAsUrlStr += to_string(coordinates.z);
	outCoordsAsUrlStr += "/";
	outCoordsAsUrlStr += to_string(coordinates.pinch);
	outCoordsAsUrlStr += "/";
	outCoordsAsUrlStr += to_string(coordinates.palmPitch);
	return true;
}


//TODO - error handling and such?
bool RobotArmNetworkClient::sendHttpRestCall(const string serverAddress, const string serverPort,
											 const string apiCommand, string& outResponseStr)
{
	try
	{
		io_service io_service;
		stringstream result;

		// Get a list of endpoints corresponding to the server name.
		tcp::resolver resolver(io_service);
		tcp::resolver::query query(serverAddress, serverPort);
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		tcp::resolver::iterator end;

		// Try each endpoint until we successfully establish a connection.
		tcp::socket socket(io_service);
		boost::system::error_code error = boost::asio::error::host_not_found;
		while (error && endpoint_iterator != end)
		{
			socket.close();
			socket.connect(*endpoint_iterator++, error);
		}
		if (error)
		{
			throw boost::system::system_error(error);
		}
			
		// Form the request. We specify the "Connection: close" header so that the
		// server will close the socket after transmitting the response. This will
		// allow us to treat all data up until the EOF as the content.
		boost::asio::streambuf request;
		ostream request_stream(&request);
		request_stream << "GET " << apiCommand << " HTTP/1.0\r\n";
		request_stream << "Host: " << serverAddress << "\r\n";
		request_stream << "Accept: */*\r\n";
		request_stream << "Connection: close\r\n\r\n";

		// Send the request.
		boost::asio::write(socket, request);

		// Read the response status line.
		boost::asio::streambuf response;
		boost::asio::read_until(socket, response, "\r\n");

		// Check that response is OK.
		istream response_stream(&response);
		string http_version;
		response_stream >> http_version;
		unsigned int status_code;
		response_stream >> status_code;
		string status_message;
		getline(response_stream, status_message);
		if (!response_stream || http_version.substr(0, 5) != "HTTP/")
		{
			cout << "Invalid response\n";
			return false;
		}
		if (status_code != 200)
		{
			cout << "Response returned with status code " << status_code << "\n";
			return false;
		}

		// Read the response headers, which are terminated by a blank line.
		boost::asio::read_until(socket, response, "\r\n\r\n");

		// Process the response headers and print it.
		string header;
		while (getline(response_stream, header) && header != "\r")
		{
			//cout << header << "\n";
		}
		//cout << "\n";

		// Write whatever content we already have to output.
		if (response.size() > 0)
		{
			result << &response;
			//cout << &response;
		}

		// Read until EOF, writing data to output as we go.
		while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error))
		{
			result << &response;
			//cout << &response;
		}
		if (error != boost::asio::error::eof)
		{
			throw boost::system::system_error(error);
		}
			
		outResponseStr = result.str();
		//cout << "RobotArmNetworkClient::sendHttpRestCall recieved result body JSON: " << result.str() << endl;
		return true;
	}
	catch (exception& e)
	{
		cout << "RobotArmNetworkClient::SendHttpRestCall - Caught exception: " << e.what() << "\n";
		return false;
	}
	
}
