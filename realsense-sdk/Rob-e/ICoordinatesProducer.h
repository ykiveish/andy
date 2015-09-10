#pragma once
#include <vector>
#include <algorithm>
#include "ICoordinatesListener.h"

using namespace std;

class ICoordinatesProducer
{
public:
	bool AddListener(ICoordinatesListener* listener)
	{
		try {
			_coordinatesListeners.push_back(listener);
		}
		catch (...)
		{
			return false;
		}

		return true;
	}

	bool RemoveListener(ICoordinatesListener* listener)
	{
		_coordinatesListeners.erase(remove(_coordinatesListeners.begin(), _coordinatesListeners.end(), listener), _coordinatesListeners.end());
	}

	void NotifyListeners(RobotCoordinates coords)
	{
		for (vector<ICoordinatesListener*>::const_iterator iter = _coordinatesListeners.begin(); iter != _coordinatesListeners.end(); ++iter)
		{
			if (*iter != 0)
			{
				(*iter)->UpdateCoordinates(coords);
			}
		}
	}

	virtual void ProduceCoordinates() = 0;

private:
	vector<ICoordinatesListener*> _coordinatesListeners;
};

