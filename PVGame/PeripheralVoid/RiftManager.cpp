#include "RiftManager.h"

//////////////////////////////////////////////////////////////////////////
// RiftManager()
//
// Set up Rift with either information from the rift if it is connected,
// or use default paramaters if the rift is not connected
//////////////////////////////////////////////////////////////////////////
RiftManager::RiftManager(char* args)
{
	//Initialize and set up managers
	OVR_UNUSED(args);
	System::Init(Log::ConfigureDefaultLog(LogMask_All));
	pManager = *DeviceManager::Create();
	pManager->SetMessageHandler(this);

	//Poll for Rift
	pHMD = *pManager->EnumerateDevices<HMDDevice>().CreateDevice();

	riftConnected = false;
	usingRift = false;
	if(pHMD)
	{
		stereo.SetFullViewport(Viewport(0,0, 1280, 800));
		//Get Information from the HMD
		if (pHMD->GetDeviceInfo(&hmd))
		{
			hmdInfo.DisplayDeviceName = hmd.DisplayDeviceName;
			hmdInfo.EyeDistance       = hmd.InterpupillaryDistance;
			hmdInfo.DistortionK[0]    = hmd.DistortionK[0];
			hmdInfo.DistortionK[1]    = hmd.DistortionK[1];
			hmdInfo.DistortionK[2]    = hmd.DistortionK[2];
			hmdInfo.DistortionK[3]    = hmd.DistortionK[3];
			hmdInfo.ScreenCenter[0]   = hmd.HScreenSize/2;
			hmdInfo.ScreenCenter[1]   = hmd.VScreenCenter;
			hmdInfo.ChromaticAberationCorrection = hmd.ChromaAbCorrection;
		}
	
		//Set up sensor
		pSensor = *pHMD->GetSensor();
		if (pSensor)
		{
			SFusion.AttachToSensor(pSensor);
			SFusion.SetDelegateMessageHandler(this);
			SFusion.SetPredictionEnabled(true);
		}

		stereo.SetHMDInfo(hmd);
	}
	else //use default values
	{
		hmdInfo.DisplayDeviceName = const_cast<char*>(stereo.GetHMDInfo().DisplayDeviceName);
			hmdInfo.EyeDistance       = stereo.GetHMDInfo().InterpupillaryDistance;
			hmdInfo.DistortionK[0]    = stereo.GetHMDInfo().DistortionK[0];
			hmdInfo.DistortionK[1]    = stereo.GetHMDInfo().DistortionK[1];
			hmdInfo.DistortionK[2]    = stereo.GetHMDInfo().DistortionK[2];
			hmdInfo.DistortionK[3]    = stereo.GetHMDInfo().DistortionK[3];
			hmdInfo.ScreenCenter[0]   = stereo.GetHMDInfo().HScreenSize/2;
			hmdInfo.ScreenCenter[1]   = stereo.GetHMDInfo().VScreenCenter;
			hmdInfo.ChromaticAberationCorrection = const_cast<float*>(stereo.GetHMDInfo().ChromaAbCorrection);
	}
	
	//Set StereoConfig Information
	stereo.SetStereoMode(Stereo_LeftRight_Multipass);
	stereo.SetDistortionFitPointVP(-1.0f, 0.0f);
	stereo.Set2DAreaFov(DegreeToRad(85.0f));
	renderScale = stereo.GetDistortionScale();
	
	leftEye = stereo.GetEyeRenderParams(StereoEye_Left);
	rightEye = stereo.GetEyeRenderParams(StereoEye_Right);
	
	//Handle different states
    if (!pHMD && !pSensor)
        detectionMessage = "Oculus Rift not detected.";
    else if (!pHMD)
        detectionMessage = "Oculus Sensor detected; HMD Display not detected.";
    else if (!pSensor)
        detectionMessage = "Oculus HMD Display detected; Sensor not detected.";
    else if (hmdInfo.DisplayDeviceName[0] == '\0')
        detectionMessage = "Oculus Sensor detected; HMD display EDID not detected.";
    else
	{
		riftConnected = true;
		MagCal.BeginAutoCalibration(SFusion);
        detectionMessage = "Oculus Rift is detected";
	}
}


RiftManager::~RiftManager(void)
{
	RemoveHandlerFromDevices();
	pManager.Clear();
	pHMD.Clear();
	pSensor.Clear();
	System::Destroy();
}

////////////////////////////////////////////////////////////////
// calcStero()
//
// poll for the rift and if it is found, set the information 
// needed to start tracking the rift.
////////////////////////////////////////////////////////////////
void RiftManager::calcStereo()
{
	//Clear current information
	RemoveHandlerFromDevices();
	pManager.Clear();
	pHMD.Clear();
	pSensor.Clear();
	System::Destroy();
	
	//Initialize and set up managers
	System::Init(Log::ConfigureDefaultLog(LogMask_All));
	pManager = *DeviceManager::Create();
	pManager->SetMessageHandler(this);
	riftConnected = false;
	pHMD = *pManager->EnumerateDevices<HMDDevice>().CreateDevice();

	//If a Rift is found, populate information
	if(pHMD)
	{
		stereo.SetFullViewport(Viewport(0,0, 1280, 800));
		//Get Information from the HMD
		if (pHMD->GetDeviceInfo(&hmd))
		{
			hmdInfo.DisplayDeviceName = hmd.DisplayDeviceName;
			hmdInfo.EyeDistance       = hmd.InterpupillaryDistance;
			hmdInfo.DistortionK[0]    = hmd.DistortionK[0];
			hmdInfo.DistortionK[1]    = hmd.DistortionK[1];
			hmdInfo.DistortionK[2]    = hmd.DistortionK[2];
			hmdInfo.DistortionK[3]    = hmd.DistortionK[3];
			hmdInfo.ScreenCenter[0]   = hmd.HScreenSize/2;
			hmdInfo.ScreenCenter[1]   = hmd.VScreenCenter;
			hmdInfo.ChromaticAberationCorrection = hmd.ChromaAbCorrection;
		}
	
		//Set up sensor
		pSensor = *pHMD->GetSensor();
		if (pSensor)
		{
			SFusion.AttachToSensor(pSensor);
			SFusion.SetDelegateMessageHandler(this);
			SFusion.SetPredictionEnabled(true);
		}

		stereo.SetHMDInfo(hmd);
	}
	else //use default information
	{
		hmdInfo.DisplayDeviceName = const_cast<char*>(stereo.GetHMDInfo().DisplayDeviceName);
			hmdInfo.EyeDistance       = stereo.GetHMDInfo().InterpupillaryDistance;
			hmdInfo.DistortionK[0]    = stereo.GetHMDInfo().DistortionK[0];
			hmdInfo.DistortionK[1]    = stereo.GetHMDInfo().DistortionK[1];
			hmdInfo.DistortionK[2]    = stereo.GetHMDInfo().DistortionK[2];
			hmdInfo.DistortionK[3]    = stereo.GetHMDInfo().DistortionK[3];
			hmdInfo.ScreenCenter[0]   = stereo.GetHMDInfo().HScreenSize/2;
			hmdInfo.ScreenCenter[1]   = stereo.GetHMDInfo().VScreenCenter;
			hmdInfo.ChromaticAberationCorrection = const_cast<float*>(stereo.GetHMDInfo().ChromaAbCorrection);
	}
	
	//Set StereoConfig information
	stereo.SetStereoMode(Stereo_LeftRight_Multipass);
	stereo.SetDistortionFitPointVP(-1.0f, 0.0f);
	stereo.Set2DAreaFov(DegreeToRad(85.0f));
	renderScale = stereo.GetDistortionScale();
	
	leftEye = stereo.GetEyeRenderParams(StereoEye_Left);
	rightEye = stereo.GetEyeRenderParams(StereoEye_Right);
	
	if (pHMD && pSensor) //Its connected!
	{
		riftConnected = true;
		MagCal.BeginAutoCalibration(SFusion);
	}
}

StereoConfig RiftManager::getStereo()
{
	return stereo;
}

StereoEyeParams RiftManager::getLeftEyeParams()
{
	return leftEye;
}

StereoEyeParams RiftManager::getRightEyeParams()
{
	return rightEye;
}

const char* RiftManager::getDetectionMessage()
{
	return detectionMessage;
}

bool RiftManager::isRiftConnected()
{
	return riftConnected;
}

HMDINFO RiftManager::getHMDInfo()
{
	return hmdInfo;
}

///////////////////////////////////////////////////////////////
// getOrientation()
//
// Handle calibration and get the orientation of the rift
//
// return a quaternion representing the rotation of the rift
///////////////////////////////////////////////////////////////
Quatf RiftManager::getOrientation()
{
	//This gets called every frame, so we can do calibration here
	if(MagCal.IsAutoCalibrating())
		MagCal.UpdateAutoCalibration(SFusion);
	else if(!MagCal.IsCalibrated())
		MagCal.BeginAutoCalibration(SFusion);

	return SFusion.GetOrientation();
}

bool RiftManager::isUsingRift()
{
	return usingRift;
}

void RiftManager::setUsingRift(bool ur)
{
	usingRift = ur;
}