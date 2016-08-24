
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

    SimpleFilter::SimpleFilter() : _buffer(renderer::texBufferPool)
	{
		_state.setBlend(false);
		_state.setCullFace(false);
		_state.setWriteDepth(false);
		_state.setDepthTest(false);
	}

	SimpleFilter::~SimpleFilter()
	{

	}

    void SimpleFilter::acquire(int)
    {
        if (_input.size() > 0 && _input[0])
        {
            _input[0]->acquire(0);

            TextureBufferPool::Key k;
            k.type = TextureBufferPool::Key::NONE;
            k.onlyTextures = false;
            k.hdr = false;
            k.res = uivec3(_input[0]->buffer()->resolution(), 1);
            _buffer.setParameter(k);

            _input[0]->release(0);
        }

        _buffer.acquire();
    }

    void SimpleFilter::release(int)
    {
        _buffer.release();
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
                {
                    _input[i]->acquire(0);
                    _input[i]->render();
                }
            }
        }
        else
        {
            for (size_t i = _input.size() ; i >= 1; --i)
            {
                if(_input[i-1] && _enableInput[i-1])
                {
                    _input[i-1]->acquire(0);
                    _input[i-1]->render();
                }
            }
        }

		_state.setShader(_filter);
        _buffer.fbo()->bind();
		_state.bind();

        int realIndex = 0;
        for (size_t i=0 ; i < _input.size() ; ++i)
		{
            if (_input[i] && _enableInput[i])
			{
                openGL.bindTextureSampler(textureSampler[TextureMode::OnlyLinearNoRepeat], realIndex);
                _input[i]->buffer()->bind(realIndex);
                realIndex++;
			}
		}

		renderer::quadMeshBuffers->draw(6, VertexMode::TRIANGLES, 1);

        if(!_invertRenderingOrder)
        {
            for (size_t i = 0; i < _input.size(); ++i)
                if(_input[i] && _enableInput[i])
                    _input[i]->release(0);
        }
        else
        {
            for (size_t i = _input.size() ; i >= 1; --i)
                if(_input[i-1] && _enableInput[i-1])
                    _input[i-1]->release(0);
        }

	}

}
}
}
