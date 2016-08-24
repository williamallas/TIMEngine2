#ifndef SIMPLEFILTER_H
#define SIMPLEFILTER_H

#include "interface/Pipeline.h"
#include "renderer/PooledBuffer.h"

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

        renderer::Texture* buffer() const override { return _buffer.buffer(0); }

        void render() override;

        void acquire(int) override;
        void release(int) override;

    private:
		renderer::Shader* _filter = nullptr;
        renderer::DrawState _state;

        renderer::PooledBuffer _buffer;
    };
}
}
}
#include "MemoryLoggerOff.h"

#endif // SIMPLEFILTER_H
