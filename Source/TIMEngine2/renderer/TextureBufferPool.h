#ifndef TEXTUREBUFFERPOOL_H
#define TEXTUREBUFFERPOOL_H

#include "FrameBuffer.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{
    class TextureBufferPool
    {
    public:
        TextureBufferPool();
        ~TextureBufferPool();

        struct Key
        {
            enum {DEFERRED_BUFFER, DEPTH_MAP_ARRAY, NONE };

            uivec3 res;
            int type = NONE;
            bool hdr = false;
            bool onlyTextures = false;

            bool operator<(const Key& k) const;
        };

        struct Buffer
        {
            FrameBuffer* fbo = nullptr;
            vector<Texture*> texs;
            int id;
        };

        Buffer getBuffer(const Key&);
        void releaseBuffer(int);

    private:
        struct InternBuffer
        {
            Buffer buffer;
            bool used;

            void clear();
        };

        static int _idGenerator;

        std::map<Key, vector<InternBuffer>> _pool;
        std::map<int, Key> _idToKey;

        static Buffer createBuffer(const Key&);
    };
}
}
#include "MemoryLoggerOff.h"

#endif // TEXTUREBUFFERPOOL_H
