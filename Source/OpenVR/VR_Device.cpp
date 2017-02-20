#include "VR_Device.h"
#include "core/core.h"

namespace tim
{
	using namespace core;

/* based on the hellowordl example form the official git*/
std::string GetTrackedDeviceString(vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop)
{
	uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, nullptr, 0, nullptr);
	if (unRequiredBufferLen == 0)
		return "";

	char *pchBuffer = new char[unRequiredBufferLen];
	pHmd->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, nullptr);

	std::string sResult = pchBuffer;
	delete[] pchBuffer;
	return sResult;
}

mat4 convertToMat4(const vr::HmdMatrix34_t &matPose)
{
	mat4 m = mat4({
		matPose.m[0][0], matPose.m[0][1], matPose.m[0][2], matPose.m[0][3],
		matPose.m[1][0], matPose.m[1][1], matPose.m[1][2], matPose.m[1][3],
		matPose.m[2][0], matPose.m[2][1], matPose.m[2][2], matPose.m[2][3],
		0.0f, 0.0f, 0.0f, 1.0f
	});
	//std::swap(m[1], m[2]);
	//m[1] *= -1;

	return m;
}

mat4 convertToMat4(const vr::HmdMatrix44_t &matPose)
{
	mat4 m = mat4({
		matPose.m[0][0], matPose.m[0][1], matPose.m[0][2], matPose.m[0][3],
		matPose.m[1][0], matPose.m[1][1], matPose.m[1][2], matPose.m[1][3],
		matPose.m[2][0], matPose.m[2][1], matPose.m[2][2], matPose.m[2][3],
		matPose.m[3][0], matPose.m[3][1], matPose.m[3][2], matPose.m[3][3],
	});
	//std::swap(m[1], m[2]);
	//m[1] *= -1;

	return m;
}

vec3 convertToVec3(const vr::HmdVector3_t &vec)
{
	return vec3({
		vec.v[0], vec.v[1], vec.v[2],
	});
}

VR_Device::VR_Device(vec2 zdist)
{
	vr::EVRInitError error;
	_hmd = vr::VR_Init(&error, vr::VRApplication_Scene);

	if (error != vr::VRInitError_None)
	{
		LOG("Unable to init VR runtime : ", vr::VR_GetVRInitErrorAsEnglishDescription(error));
		return;
	}

	_compositor = vr::VRCompositor();
	if (!_compositor)
		LOG("Compositor initialization failed.\n");

	std::string strTrackingSystem = "No Driver";
	std::string strManufacturer = "No Display";

	strTrackingSystem = GetTrackedDeviceString(_hmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
	strManufacturer   = GetTrackedDeviceString(_hmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_ManufacturerName_String);

	LOG("openvr initialized on ", strTrackingSystem, " by ", strManufacturer, "\n");

	_hmdCamera._projection[LEFT] = convertToMat4(_hmd->GetProjectionMatrix(vr::Eye_Left, zdist.x(), zdist.y(), vr::API_OpenGL));
	_hmdCamera._projection[RIGHT] = convertToMat4(_hmd->GetProjectionMatrix(vr::Eye_Right, zdist.x(), zdist.y(), vr::API_OpenGL));

	_hmdCamera._hmdToEye[RIGHT] = convertToMat4(_hmd->GetEyeToHeadTransform(vr::Eye_Left));
	_hmdCamera._hmdToEye[LEFT] = convertToMat4(_hmd->GetEyeToHeadTransform(vr::Eye_Right));
}

VR_Device::~VR_Device()
{
	vr::VR_Shutdown();
}

uivec2 VR_Device::hmdResolution() const
{
	if (!_hmd) return uivec2();

	uint32_t x=0, y=0;
	_hmd->GetRecommendedRenderTargetSize(&x, &y);

	return uivec2(x, y);
}

void VR_Device::sync()
{
    if(_compositor)
        _compositor->WaitGetPoses(_vrTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);
}

void VR_Device::update(bool useWaitgetPoses)
{
	if (!_compositor) return;

    if(useWaitgetPoses)
        _compositor->WaitGetPoses(_vrTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);
    else
    {         
        float fSecondsSinceLastVsync;
        _hmd->GetTimeSinceLastVsync( &fSecondsSinceLastVsync, NULL );

        float fDisplayFrequency = _hmd->GetFloatTrackedDeviceProperty( vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_DisplayFrequency_Float );
        float fFrameDuration = 1.f / fDisplayFrequency;
        float fVsyncToPhotons = _hmd->GetFloatTrackedDeviceProperty( vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SecondsFromVsyncToPhotons_Float );

        float fPredictedSecondsFromNow = fFrameDuration - fSecondsSinceLastVsync + fVsyncToPhotons;

        //std::cout<< "Predicted ms from photons:" << fPredictedSecondsFromNow*1000 << "ms\n";
        _hmd->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseStanding, fPredictedSecondsFromNow, _vrTrackedDevicePose, vr::k_unMaxTrackedDeviceCount);
    }

    int controllerFound=0;
    _isControllerConnected[0] = false;
    _isControllerConnected[1] = false;

    for (uint nDevice = 0 ; nDevice < vr::k_unMaxTrackedDeviceCount ; ++nDevice)
	{
		if (_vrTrackedDevicePose[nDevice].bPoseIsValid)
		{
			_devicePose[nDevice] = convertToMat4(_vrTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);
            _deviceVel[nDevice] = convertToVec3(_vrTrackedDevicePose[nDevice].vVelocity);
			std::swap(_devicePose[nDevice][1], _devicePose[nDevice][2]);
			_devicePose[nDevice][1] *= -1;
		}

        if(nDevice > vr::k_unTrackedDeviceIndex_Hmd && controllerFound < 2)
        {
            if(_hmd->GetTrackedDeviceClass(nDevice) == vr::TrackedDeviceClass_Controller && _hmd->IsTrackedDeviceConnected(nDevice))
            {
                _controller[controllerFound] = nDevice;
                _isControllerConnected[controllerFound++] = true;
            }
        }
	}

	if (_vrTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
	{
        if(_hmd->IsTrackedDeviceConnected(vr::k_unTrackedDeviceIndex_Hmd))
            _hmdConnected = true;
        else
            _hmdConnected = false;

		_hmdCamera._hmdMatrix = _devicePose[vr::k_unTrackedDeviceIndex_Hmd].inverted();
		_hmdCamera._vel = convertToVec3(_vrTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].vVelocity);
		_hmdCamera._angVel = convertToVec3(_vrTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].vAngularVelocity);

		_hmdCamera._eyeView[LEFT] = _hmdCamera._hmdToEye[LEFT] * _hmdCamera._hmdMatrix;
		_hmdCamera._eyeView[RIGHT] = _hmdCamera._hmdToEye[RIGHT] * _hmdCamera._hmdMatrix;
	}
    else
        _hmdConnected = false;

}

}

