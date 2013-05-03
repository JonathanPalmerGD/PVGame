#pragma once
#include "LibOVR\Include\OVR.h"

using namespace OVR;

struct HMDINFO
{
	char* DisplayDeviceName;
	float EyeDistance;
	float DistortionK[4];
};

class RiftManager : public MessageHandler
{
private:
	bool riftConnected;
	Ptr<DeviceManager> pManager;
	Ptr<HMDDevice> pHMD;
	HMDInfo hmd;

	Ptr<SensorDevice> pSensor;
	SensorFusion SFusion;

	HMDINFO hmdInfo;

	int detectionResult;
	const char* detectionMessage;

	Matrix4f leftMatrix;
	Matrix4f rightMatrix;
public:
	RiftManager(void);
	~RiftManager(void);

	bool isRiftConnected();
	HMDINFO getHMDInfo();
	const char* getDetectionMessage();

	void calcMatricies();
	Matrix4f getLeftMatrix();
	Matrix4f getRightMatrix();

	Quatf getOrientation();
};

