#pragma once
#include "LibOVR\Include\OVR.h"
#include "LibOVR\Src\Util\Util_Render_Stereo.h"

using namespace OVR;
using namespace OVR::Util::Render;

struct HMDINFO
{
	char* DisplayDeviceName;
	float EyeDistance;
	float DistortionK[4];
	float ScreenCenter[2];
	float* ChromaticAberationCorrection;
};

class RiftManager : public MessageHandler
{
private:
	bool riftConnected;
	bool usingRift;
	Ptr<DeviceManager> pManager;
	Ptr<HMDDevice> pHMD;
	HMDInfo hmd;

	Ptr<SensorDevice> pSensor;
	SensorFusion SFusion;

	HMDINFO hmdInfo;

	int detectionResult;
	const char* detectionMessage;

	StereoConfig stereo;
	StereoEyeParams leftEye;
	StereoEyeParams rightEye;
	float renderScale;

	Matrix4f leftMatrix;
	Matrix4f rightMatrix;
public:
	RiftManager(void);
	~RiftManager(void);

	bool isRiftConnected();
	HMDINFO getHMDInfo();
	const char* getDetectionMessage();

	void calcMatricies(Matrix4f viewCenterMat);
	void calcMatriciesNoRift();
	Matrix4f getLeftMatrix();
	Matrix4f getRightMatrix();

	StereoEyeParams getLeftEyeParams();
	StereoEyeParams getRightEyeParams();
	void calcStereo();
	StereoConfig getStereo();

	Quatf getOrientation();

	bool isUsingRift();
	void setUsingRift(bool);
};


