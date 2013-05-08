#include "RiftManager.h"


RiftManager::RiftManager(void)
{
	System::Init(Log::ConfigureDefaultLog(LogMask_All));
	pManager = *DeviceManager::Create();
	pManager->SetMessageHandler(this);

	pHMD = *pManager->EnumerateDevices<HMDDevice>().CreateDevice();

	riftConnected = false;

	if(pHMD)
	{
		stereo.SetFullViewport(Viewport(0,0, 800, 600));
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
		}
	}
	
	stereo.SetHMDInfo(hmd);
	stereo.SetFullViewport(Viewport(0,0, hmd.HResolution, hmd.VResolution));
	stereo.SetStereoMode(Stereo_LeftRight_Multipass);
	stereo.SetDistortionFitPointVP(-1.0f, 0.0f);
	renderScale = stereo.GetDistortionScale();
	
	leftEye = stereo.GetEyeRenderParams(StereoEye_Left);
	rightEye = stereo.GetEyeRenderParams(StereoEye_Right);
	
	
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
        detectionMessage = "Oculus Rift is detected";
	}
}


RiftManager::~RiftManager(void)
{
	pManager.Clear();
	pHMD.Clear();
	pSensor.Clear();
	System::Destroy();
}

void RiftManager::calcStereo()
{
	stereo.SetHMDInfo(hmd);
	stereo.SetFullViewport(Viewport(0,0, hmd.HResolution, hmd.VResolution));
	stereo.SetStereoMode(Stereo_LeftRight_Multipass);
	stereo.SetDistortionFitPointVP(-1.0f, 0.0f);
	renderScale = stereo.GetDistortionScale();

	
	leftEye = stereo.GetEyeRenderParams(StereoEye_Left);
	rightEye = stereo.GetEyeRenderParams(StereoEye_Right);
}

StereoConfig RiftManager::getStereo()
{
	return stereo;
}

StereoEyeParams RiftManager::getLeftEyeParams()
{
	leftEye = stereo.GetEyeRenderParams(StereoEye_Left);
	return leftEye;
}

StereoEyeParams RiftManager::getRightEyeParams()
{
	rightEye = stereo.GetEyeRenderParams(StereoEye_Right);
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

void RiftManager::calcMatricies(Matrix4f viewCenterMat)
{
	// Compute Aspect Ratio. Stereo mode cuts width in half.
	float aspectRatio = float(hmd.HResolution * 0.5f) / float(hmd.VResolution);

	// Compute Vertical FOV based on distance.
	float halfScreenDistance = (hmd.VScreenSize / 2);
	float yfov = 2.0f * atan(halfScreenDistance/hmd.EyeToScreenDistance);

	// Post-projection viewport coordinates range from (-1.0, 1.0), with the
	// center of the left viewport falling at (1/4) of horizontal screen size.
	// We need to shift this projection center to match with the lens center.
	// We compute this shift in physical units (meters) to correct
	// for different screen sizes and then rescale to viewport coordinates.
	float viewCenter = hmd.HScreenSize * 0.25f;
	float eyeProjectionShift = viewCenter - hmd.LensSeparationDistance*0.5f;
	float projectionCenterOffset = 4.0f * eyeProjectionShift / hmd.HScreenSize;

	// Projection matrix for the "center eye", which the left/right matrices are based on.
	Matrix4f projCenter = Matrix4f::PerspectiveRH(yfov, aspectRatio, 0.01f, 1000.0f);
	Matrix4f projLeft = Matrix4f::Translation(projectionCenterOffset, 0, 0) * projCenter;
	Matrix4f projRight = Matrix4f::Translation(-projectionCenterOffset, 0, 0) * projCenter;

	// View transformation translation in world units.
	float halfIPD = hmd.InterpupillaryDistance * 0.5f;
	leftMatrix  = Matrix4f::Translation(halfIPD, 0, 0) * viewCenter;
	rightMatrix = Matrix4f::Translation(-halfIPD, 0, 0) * viewCenter;
	
	//leftMatrix  *= viewCenterMat;
	//rightMatrix *= viewCenterMat;
}

void RiftManager::calcMatriciesNoRift()
{
	// Compute Aspect Ratio. Stereo mode cuts width in half.
	float aspectRatio = float(800 * 0.5f) / float(600);

	// Compute Vertical FOV based on distance.
	float halfScreenDistance = (0.9359997f / 2);
	float yfov = 2.0f * atan(halfScreenDistance / 0.04100001f);

	// Post-projection viewport coordinates range from (-1.0, 1.0), with the
	// center of the left viewport falling at (1/4) of horizontal screen size.
	// We need to shift this projection center to match with the lens center.
	// We compute this shift in physical units (meters) to correct
	// for different screen sizes and then rescale to viewport coordinates.
	float viewCenter = 0.14975999f * 0.25f;
	float eyeProjectionShift = viewCenter - 0.063500002f * 0.5f;
	float projectionCenterOffset = 4.0f * eyeProjectionShift / 0.14975999f;

	// Projection matrix for the "center eye", which the left/right matrices are based on.
	Matrix4f projCenter = Matrix4f::PerspectiveRH(yfov, /*aspect*/800.0f / 600.0f, 0.3f, 1000.0f);
	Matrix4f projLeft = Matrix4f::Translation(projectionCenterOffset, 0, 0) * projCenter;
	Matrix4f projRight = Matrix4f::Translation(-projectionCenterOffset, 0, 0) * projCenter;

	// View transformation translation in world units.
	float halfIPD = 0.064000003 * 0.5f;
	leftMatrix  = Matrix4f::Translation(halfIPD, 0, 0) * viewCenter;
	rightMatrix = Matrix4f::Translation(-halfIPD, 0, 0) * viewCenter;

}

Matrix4f RiftManager::getLeftMatrix()
{
	return leftMatrix;
}

Matrix4f RiftManager::getRightMatrix()
{
	return rightMatrix;
}

Quatf RiftManager::getOrientation()
{
	return SFusion.GetOrientation();
}