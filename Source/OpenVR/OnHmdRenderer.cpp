#include "OnHmdRenderer.h"
#include "core/core.h"
#undef interface; 

namespace tim
{
	using namespace core;

	OnHmdRenderer::OnHmdRenderer() : interface::Pipeline::TerminalNode()
	{
		_stateDrawQuad.setCullFace(false);
		_stateDrawQuad.setDepthTest(false);
		_stateDrawQuad.setWriteDepth(false);
		_stateDrawQuad.setShader(renderer::drawQuadShader);

		//renderer::FrameBuffer _fboBuffer[2];
	}

	OnHmdRenderer::~OnHmdRenderer()
	{
		delete _textureBuffer[0];
		delete _textureBuffer[1];
	}

	void OnHmdRenderer::prepare()
	{
		if (!tryPrepare()) return;

		for (uint i = 0; i<_input.size(); ++i)
		{
			if (_input[i])
				_input[i]->prepare();
		}
	}

	void OnHmdRenderer::render()
	{
		if (!tryRender()) return;

		for (uint i = 0; i<_input.size(); ++i)
		{
			if (_input[i])
				_input[i]->render();
		}

		if (!_textureBuffer[0] && _input[0] && _input[1])
		{
			renderer::Texture::GenTexParam param;
			param.size = uivec3(_input[0]->buffer()->resolution(), 0);
			param.nbLevels = 1;
			param.format = renderer::Texture::Format::RGBA8;
			_textureBuffer[VR_Device::LEFT] = renderer::Texture::genTexture2D(param);
			_textureBuffer[VR_Device::RIGHT] = renderer::Texture::genTexture2D(param);

			_fboBuffer[VR_Device::LEFT].attachTexture(0, _textureBuffer[VR_Device::LEFT]);
			_fboBuffer[VR_Device::RIGHT].attachTexture(0, _textureBuffer[VR_Device::RIGHT]);

			_fboInput[VR_Device::LEFT].attachTexture(0, _input[VR_Device::LEFT]->buffer());
			_fboInput[VR_Device::RIGHT].attachTexture(0, _input[VR_Device::RIGHT]->buffer());
		}

		_fboInput[VR_Device::LEFT].copyTo(_fboBuffer[VR_Device::LEFT]);
		_fboInput[VR_Device::RIGHT].copyTo(_fboBuffer[VR_Device::RIGHT]);


		if (_drawOnScreen >= 0 && _input.size() > 0)
		{
			renderer::openGL.bindFrameBuffer(0);
			_stateDrawQuad.bind();

			renderer::openGL.bindTextureSampler(renderer::textureSampler[renderer::TextureMode::NoFilter], 0);
			renderer::openGL.bindTexture(_textureBuffer[std::min(_drawOnScreen, 1)]->id(), GL_TEXTURE_2D, 0);
			//renderer::openGL.bindTexture(_input[std::min(_drawOnScreen, 2)]->buffer()->id(), GL_TEXTURE_2D, 0);

			renderer::quadMeshBuffers->draw(6, renderer::VertexMode::TRIANGLES, 1);
		}

		// Submit to hmd
		if (_device->isInit() && _input[0] && _input[1])
		{
			vr::Texture_t leftEyeTexture = { (void*)_textureBuffer[VR_Device::LEFT]->id(), vr::API_OpenGL, vr::ColorSpace_Linear };
			vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);

			vr::Texture_t rightEyeTexture = { (void*)_textureBuffer[VR_Device::RIGHT]->id(), vr::API_OpenGL, vr::ColorSpace_Linear };
			vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
		}
		//
	}

}

