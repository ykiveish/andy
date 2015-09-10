#include "CoordinatesProducerRS.h"
#include "OpenCVCameraClient.h"

CoordinatesProducerRS::CoordinatesProducerRS()
{
	InitRealSenseHandTracking();
}


CoordinatesProducerRS::~CoordinatesProducerRS()
{
}

void CoordinatesProducerRS::ProduceCoordinates()
{
	
	#if 0
		while (true)
		{
			RobotCoordinates* robotCoordinates = new RobotCoordinates(0, 0, 0, 0, 0);
			NotifyListeners(*robotCoordinates);
			if (cv::waitKey(30) >= 0) break;
		}
	#else



	g_default = !(g_alerts && g_gestures && g_skeleton);
	pxcI32 numOfHands = 0;

	// First Initializing the sense manager
	if (g_senseManager->Init() == PXC_STATUS_NO_ERROR)
	{
		std::printf("\nPXCSenseManager Initializing OK\n========================\n");

		if (g_default)
		{
			std::printf("Number of hands: %d\n", numOfHands);
		}

		int count = 0;
		float pitchZdelta = 0;
		float convertedPitch = 0;

		// Acquiring frames from input device
		while (g_senseManager->AcquireFrame(true) == PXC_STATUS_NO_ERROR && !g_stop)
		{
			RobotCoordinates* robotCoordinates = NULL;
			// Get current hand outputs
			if (g_handDataOutput->Update() == PXC_STATUS_NO_ERROR)
			{				
				// Display alerts
				if (g_alerts)
				{
					PXCHandData::AlertData alertData;
					for (int i = 0; i < g_handDataOutput->QueryFiredAlertsNumber(); ++i)
					{
						if (g_handDataOutput->QueryFiredAlertData(i, alertData) == PXC_STATUS_NO_ERROR)
						{
							std::printf("%s was fired at frame %d \n", Definitions::AlertToString(alertData.label).c_str(), alertData.frameNumber);
						}
					}
				}

				// Display gestures
				if (g_gestures)
				{
					PXCHandData::GestureData gestureData;
					for (int i = 0; i < g_handDataOutput->QueryFiredGesturesNumber(); ++i)
					{
						if (g_handDataOutput->QueryFiredGestureData(i, gestureData) == PXC_STATUS_NO_ERROR)
						{
							std::wprintf(L"%s, Gesture: %s was fired at frame %d \n", Definitions::GestureStateToString(gestureData.state), gestureData.name, gestureData.frameNumber);
						}
					}
				}

				// Display joints
				if (g_skeleton)
				{
					PXCHandData::IHand *hand;
					PXCHandData::JointData jointData;
					PXCHandData::JointData jointMiddleBase;
					PXCHandData::JointData jointWrist;
					for (int i = 0; i < g_handDataOutput->QueryNumberOfHands(); ++i)
					{
						g_handDataOutput->QueryHandData(PXCHandData::ACCESS_ORDER_BY_TIME, i, hand);
						std::string handSide = "Unknown Hand";
						handSide = hand->QueryBodySide() == PXCHandData::BODY_SIDE_LEFT ? "Left Hand" : "Right Hand";

						
						if (handSide == "Left Hand")
						{
							continue;
						}
						else if (count < 5)
						{
							count++;
							continue;
						}
						else
						{
							count = 0;
						}

						//std::printf("%s\n==============\n", handSide.c_str());
						int j = 1;
						if (hand->QueryTrackedJoint((PXCHandData::JointType)j, jointData) == PXC_STATUS_NO_ERROR)
						{
							//std::printf("     %s)\tX: %f, Y: %f, Z: %f O: %d\n", Definitions::JointToString((PXCHandData::JointType)j).c_str(), 
							//	jointData.positionWorld.x, jointData.positionWorld.y, jointData.positionWorld.z, 
							//	hand->QueryOpenness());
							
							hand->QueryTrackedJoint(PXCHandData::JointType::JOINT_MIDDLE_BASE, jointMiddleBase);
							hand->QueryTrackedJoint(PXCHandData::JointType::JOINT_WRIST, jointWrist);
							pitchZdelta = jointMiddleBase.positionWorld.z - jointWrist.positionWorld.z;
							
							//std::printf(" CoordsProducerRS:  jointMiddleBase.z: %f, jointWrist.z: %f, pitchZdelta: %f\n", jointMiddleBase.positionWorld.z, jointWrist.positionWorld.z, pitchZdelta);
							if (pitchZdelta > 100) pitchZdelta = 100;
							convertedPitch = ConverPitchZeroToNinty(pitchZdelta, 0, -0.07, 100, 0) + 45.0;
							//std::printf(" CoordsProducerRS:  convertedPitch: %f\n", convertedPitch);
							//////////////////////////////////
							robotCoordinates = new RobotCoordinates(jointData.positionWorld.x, jointData.positionWorld.y, jointData.positionWorld.z, hand->QueryOpenness(), convertedPitch);

						}
					}
				}

				// Display number of hands
				if (g_default)
				{
					if (numOfHands != g_handDataOutput->QueryNumberOfHands())
					{
						numOfHands = g_handDataOutput->QueryNumberOfHands();
						std::printf("Number of hands: %d\n", numOfHands);
					}
				}

			} // end if update

			g_senseManager->ReleaseFrame();
			if (robotCoordinates)
			{
				NotifyListeners(*robotCoordinates);
				delete(robotCoordinates);
				robotCoordinates = NULL;
			}
		} // end while acquire frame
	} // end if Init

	else
	{
		releaseAll();
		std::printf("Failed Initializing PXCSenseManager\n");
		return;
	}

	#endif
}

float CoordinatesProducerRS::ConverPitchZeroToNinty(float oldValue, float oldMin, float oldMax, float newMin, float newMax)
{
	float convertedValue = (((oldValue - oldMin) * (newMax - newMin)) / (oldMax - oldMin)) + newMin;
	if (convertedValue > 90) convertedValue = 90;
	if (convertedValue < 0) convertedValue = 0;
	return convertedValue;
}


int CoordinatesProducerRS::InitRealSenseHandTracking()
{
	// Setup
	g_session = PXCSession::CreateInstance();
	if (!g_session)
	{
		std::printf("Failed Creating PXCSession\n");
		return -1;
	}

	/* Optional steps to send feedback to Intel Corporation to understand how often each SDK sample is used. */
	PXCMetadata * md = g_session->QueryInstance<PXCMetadata>();
	if (md) {
		pxcCHAR sample_name[] = L"Hands Console";
		md->AttachBuffer(PXCSessionService::FEEDBACK_SAMPLE_INFO, (pxcBYTE*)sample_name, sizeof(sample_name));
	}

	g_senseManager = g_session->CreateSenseManager();
	if (!g_senseManager)
	{
		releaseAll();
		std::printf("Failed Creating PXCSenseManager\n");
		return -1;
	}

	if (g_senseManager->EnableHand() != PXC_STATUS_NO_ERROR)
	{
		releaseAll();
		std::printf("Failed Enabling Hand Module\n");
		return -1;
	}

	g_handModule = g_senseManager->QueryHand();
	if (!g_handModule)
	{
		releaseAll();
		std::printf("Failed Creating PXCHandModule\n");
		return -1;
	}

	g_handDataOutput = g_handModule->CreateOutput();
	if (!g_handDataOutput)
	{
		releaseAll();
		std::printf("Failed Creating PXCHandData\n");
		return -1;
	}

	g_handConfiguration = g_handModule->CreateActiveConfiguration();
	if (!g_handConfiguration)
	{
		releaseAll();
		std::printf("Failed Creating PXCHandConfiguration\n");
		return -1;
	}

	// Set full tracking mode to get the hand openess
	g_handConfiguration->SetTrackingMode(PXCHandData::TRACKING_MODE_FULL_HAND);

	// Apply configuration setup
	g_handConfiguration->ApplyChanges();

	// run sequences as fast as possible
	if (!g_live)
	{
		g_senseManager->QueryCaptureManager()->SetFileName(g_sequencePath.c_str(), false);
		g_senseManager->QueryCaptureManager()->SetRealtime(false);
	}
	if (g_handConfiguration)
	{
		g_handConfiguration->Release();
		g_handConfiguration = NULL;
	}

	return 0;
}

void CoordinatesProducerRS::releaseAll()
{
	if (g_senseManager)
	{
		g_senseManager->Close();
		g_senseManager->Release();
		g_senseManager = NULL;
	}
	if (g_session)
	{
		g_session->Release();
		g_session = NULL;
	}
}
