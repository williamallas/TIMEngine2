#ifndef SIMPLEFILTER_H
#define SIMPLEFILTER_H

#include "interface/Pipeline.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace interface
{
namespace pipeline
{
    class SimpleFilter : public Pipeline::InOutBufferNode
    {
    public:
		SimpleFilter();
		~SimpleFilter();

		void setShader(renderer::Shader* sh) { _filter = sh; }

        renderer::Texture* buffer() const override { return _buffer; }

        void render() override;

    private:
		renderer::Shader* _filter = nullptr;
		renderer::Texture* _buffer = nullptr;
		renderer::DrawState _state;
		renderer::FrameBuffer _fbo;
    };
}
}
}
#include "MemoryLoggerOff.h"

#endif // SIMPLEFILTER_H
