
#include "SimpleFilter.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
    using namespace renderer;
namespace interface
{
namespace pipeline
{

	SimpleFilter::SimpleFilter()
	{
		_state.setBlend(false);
		_state.setCullFace(false);
		_state.setWriteDepth(false);
		_state.setDepthTest(false);
	}

	SimpleFilter::~SimpleFilter()
	{
		delete _buffer;
	}

	void SimpleFilter::render()
	{
		if (!_filter || _input.empty() || _input[0] == nullptr)
			return;

        if(!_invertRenderingOrder)
        {
            for (size_t i = 0; i < _input.size(); ++i)
            {
                if(_input[i] && _enableInput[i])
                    _input[i]->render();
            }
        }
        else
        {
            for (size_t i = _input.size() ; i >= 1; --i)
            {
                if(_input[i-1] && _enableInput[i-1])
                    _input[i-1]->render();
            }
        }

		if (_buffer == nullptr || _buffer->resolution() != _input[0]->buffer()->resolution())
		{
			if (_buffer)
				delete _buffer;

			renderer::Texture::GenTexParam param;
			param.size = uivec3(_input[0]->buffer()->resolution(), 0);
			param.nbLevels = 1;
			param.format = renderer::Texture::Format::RGBA8;
			_buffer = renderer::Texture::genTexture2D(param);
			_fbo.attachTexture(0, _buffer);
		}

		_state.setShader(_filter);
		_fbo.bind();
		_state.bind();

        for (size_t i=0 ; i < _input.size() ; ++i)
		{
            if (_input[i] && _enableInput[i])
			{
                openGL.bindTextureSampler(textureSampler[TextureMode::OnlyLinearNoRepeat], i);
                _input[i]->buffer()->bind(i);
			}
		}

		renderer::quadMeshBuffers->draw(6, VertexMode::TRIANGLES, 1);
	}

}
}
}
