#ifndef ON_HMD_RENDERER_H
#define ON_HMD_RENDERER_H

#include "VR_Device.h"
#include "interface/Pipeline.h"
#undef interface

namespace tim {
	class OnHmdRenderer : public interface::Pipeline::TerminalNode
	{
	public:
		OnHmdRenderer();
		~OnHmdRenderer();

		void setVRDevice(VR_Device* device) { _device = device; }

		void prepare() override;
		void render() override;

		void setDrawOnScreen(int index) { _drawOnScreen = index; }
        void setScreenResolution(uivec2 res) { _screenResolution = res; }
        void setShader(renderer::Shader* sh) { _stateDrawQuad.setShader(sh); }
        void setInvertEyes(bool e) { _invertEyes = e; }

	private:
		VR_Device* _device;
		int _drawOnScreen = 0;
        bool _invertEyes = false;
        uivec2 _screenResolution = {1600,900};

		renderer::DrawState _stateDrawQuad;

		renderer::FrameBuffer _fboBuffer[2];
		renderer::FrameBuffer _fboInput[2];
		renderer::Texture* _textureBuffer[2] = { nullptr, nullptr };
	};
}

#endif // ON_HMD_RENDERER_H
