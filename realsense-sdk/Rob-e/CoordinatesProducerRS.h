#pragma once
#include "ICoordinatesProducer.h"
#include "pxcsensemanager.h"
#include "pxcsession.h"
#include "pxccapture.h"
#include "pxchandmodule.h"
#include "pxchanddata.h"
#include "pxchandconfiguration.h"
#include <windows.h>
#include <iostream>

#include "pxcsensemanager.h"
#include "pxcmetadata.h"
#include "service/pxcsessionservice.h"

#include "pxchanddata.h"
#include "pxchandconfiguration.h"

#include "Definitions.h"


class CoordinatesProducerRS : public ICoordinatesProducer
{
private:
	bool g_live = true; // true - Working in live camera mode, false - sequence mode
	bool g_gestures = false; // Writing gesture data to console ouput
	bool g_alerts = true; // Writing alerts data to console ouput
	bool g_skeleton = true; // Writing skeleton data (22 joints) to console ouput
	bool g_default = false; // Writing hand type to console ouput
	bool g_stop = false; // user closes application

	std::wstring g_sequencePath;

	PXCSession *g_session;
	PXCSenseManager *g_senseManager;
	PXCHandModule *g_handModule;
	PXCHandData *g_handDataOutput;
	PXCHandConfiguration *g_handConfiguration;

	float maxRangeValue = 1000;

	int InitRealSenseHandTracking();
	float ConverPitchZeroToNinty(float oldValue, float oldMin, float oldMax, float newMin, float newMax);

public:
	CoordinatesProducerRS();
	~CoordinatesProducerRS();
	void ProduceCoordinates();
	//CameraBoundaries getCameraBoundaries();
	void releaseAll();
};

