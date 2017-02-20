#ifndef VERTEXFORMAT_H_INCLUDED
#define VERTEXFORMAT_H_INCLUDED

#include "core/Vector.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{
    static constexpr uint VNCT_BYTE_SIZE = 44;

    enum VertexFormat
    {
        V=0,
        VN,
        VNC,
        VNCT,
        VC,
        VEC1,
    };

    enum VertexMode
    {
        TRIANGLES=0,
        TRIANGLE_STRIP,
        LINES,
        LINE_STRIP,
        POINTS,
        QUADS,
    };

    struct VN_Vertex
    {
        vec3 v, n;
        bool operator<(const VN_Vertex& in) const
        {
            if(v < in.v)
                return true;
            else if(v != in.v)
                return false;

            if(n < in.n)
                return true;
            else if(n != in.n)
                return false;

            return false;
        }

        bool operator==(const VN_Vertex& in) const
        {
            return v==in.v && n==in.n;
        }
    };

    struct VNC_Vertex
    {
        vec3 v, n; vec2 c;
        bool operator<(const VNC_Vertex& in) const
        {
            if(v < in.v)
                return true;
            else if(v != in.v)
                return false;

            if(n < in.n)
                return true;
            else if(n != in.n)
                return false;

            if(c < in.c)
                return true;
            else if(c != in.c)
                return false;

            return false;
        }

        bool operator==(const VNC_Vertex& in) const
        {
            return v==in.v && n==in.n && c==in.c;
        }
    };

    struct VNCT_Vertex
    {
        vec3 v, n;
        vec2 c;
        vec3 t;
        bool operator<(const VNCT_Vertex& in) const
        {
            vec3 v, n; vec2 c; vec3 t;
            if(v < in.v)
                return true;
            else if(v != in.v)
                return false;

            if(n < in.n)
                return true;
            else if(n != in.n)
                return false;

            if(c < in.c)
                return true;
            else if(c != in.c)
                return false;

            if(t < in.t)
                return true;
            else if(t != in.t)
                return false;

            return false;
        }

        bool operator==(const VNCT_Vertex& in) const
        {
            return v==in.v && n==in.n && c==in.c && t==in.t;
        }
    };

    struct VC_Vertex
    {
        vec3 v; vec2 c;
        bool operator<(const VC_Vertex& in) const
        {
            vec3 v; vec2 c;
            if(v < in.v)
                return true;
            else if(v != in.v)
                return false;

            if(c < in.c)
                return true;
            else if(c != in.c)
                return false;

            return false;
        }

        bool operator==(const VNCT_Vertex& in) const
        {
            return v==in.v && c==in.c;
        }
    };

    inline uint vertexFormatSize(VertexFormat format)
    {
        switch(format)
        {
            case VertexFormat::V: return 3;
            case VertexFormat::VC: return 5;
            case VertexFormat::VN: return 6;
            case VertexFormat::VNC: return 8;
            case VertexFormat::VNCT: return 11;
            case VertexFormat::VEC1: return 1;
            default: return 0;
        }
    }

    inline uint vertexFormatOffset(VertexFormat format)
    {
        switch(format)
        {
            case VertexFormat::V: return 1;
            case VertexFormat::VC: return 2;
            case VertexFormat::VN: return 2;
            case VertexFormat::VNC: return 3;
            case VertexFormat::VNCT: return 4;
            case VertexFormat::VEC1: return 1;
            default: return 0;
        }
    }
}
}
#include "MemoryLoggerOff.h"

#endif // VERTEXFORMAT_H_INCLUDED
