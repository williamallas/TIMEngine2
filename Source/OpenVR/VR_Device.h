#ifndef VR_DEVICE_H
#define VR_DEVICE_H

#include <openvr_mingw.hpp>
#include "core/Matrix.h"

#include "renderer/FrameBuffer.h"

namespace tim {
	class VR_Device
	{
		

	public:
		enum { LEFT = 0, RIGHT = 1 };

		class HmdCamera
		{
		public:
			friend class VR_Device;
			const mat4& hmdView() const { return _hmdMatrix;  }

			const mat4& eyeProjection(int eye) const { return _projection[eye]; }
			const mat4& eyeView(int eye) const { return _eyeView[eye]; }

		private:
			HmdCamera() = default;
			mat4 _hmdMatrix;
			vec3 _vel, _angVel;

			mat4 _projection[2];
			mat4 _hmdToEye[2];
			mat4 _eyeView[2];
		};

		VR_Device(vec2 zdist = { 0.1f, 1000.f });
		~VR_Device();

		uivec2 hmdResolution() const;
		bool isInit() const { return _hmd != nullptr;  }
		void update();
		HmdCamera& camera() { return _hmdCamera;  }
        const HmdCamera& camera() const { return _hmdCamera;  }

        const core::mat4& controllerPose(int id) const { return _devicePose[_controller[id]]; }
        const core::vec3& controllerVel(int id) const { return _deviceVel[_controller[id]]; }
        bool isControllerConnected(int id) const { return _isControllerConnected[id]; }

	protected:
		vr::IVRSystem* _hmd = nullptr;
		vr::IVRCompositor* _compositor = nullptr;

		core::mat4 _devicePose[vr::k_unMaxTrackedDeviceCount];
        core::vec3 _deviceVel[vr::k_unMaxTrackedDeviceCount];
		vr::TrackedDevicePose_t _vrTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];

		HmdCamera _hmdCamera;
        uint _controller[2];
        bool _isControllerConnected[2] = {false, false};
	};
}

#endif // VR_DEVICE_H
