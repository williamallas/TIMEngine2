#ifndef IMAGE_ALGORITHM_H_INCLUDED
#define IMAGE_ALGORITHM_H_INCLUDED

#include "Vector.h"
#include "PascaleTriangle.h"

#include "MemoryLoggerOn.h"
namespace tim
{
namespace core
{
    template <class T>
    class ImageAlgorithm
    {
    public:
        ImageAlgorithm() : _data(nullptr), _size(0,0) {}
        ImageAlgorithm(T* && data, uivec2 size) : _data(data), _size(size) {}
        ImageAlgorithm(T*, uivec2);
        ImageAlgorithm(uivec2);
        ImageAlgorithm(const ImageAlgorithm&);
        ImageAlgorithm(ImageAlgorithm&&);
        ~ImageAlgorithm() { clear(); }

        ImageAlgorithm& operator=(const ImageAlgorithm&);
        ImageAlgorithm& operator=(ImageAlgorithm&&);

        uivec2 size() const { return _size; }
        bool empty() const { return _data == nullptr; }

        T* data() const { return _data; }
        T* detachData();

        void set(uint, uint, const T&);

        const T& get(uint x, uint y) const { return safe_get({x,y}); }
        T& get(uint x, uint y) { return safe_get({x,y}); }

        const T& clamp_get(int x, int y) const;
        T& clamp_get(int x, int y);

        ImageAlgorithm blur3x3() const;
        template <uint KS> ImageAlgorithm blur() const;


    private:
        T* _data;
        uivec2 _size;

        void build(uivec2);
        void clear();

        const T& safe_get(uivec2) const;
        T& safe_get(uivec2);
        const T& get(uivec2) const;
        T& get(uivec2);

        bool check(uivec2 v) const { return v.x() < _size.x() && v.y() < _size.y(); }
    };

    template <class T>
    ImageAlgorithm<T>::ImageAlgorithm(T* dat, uivec2 s) : _data(nullptr), _size(0,0)
    {
        if(!dat) return;

        build(s);
        for(uint i=0 ; i<_size.x() ; ++i)
            for(uint j=0 ; j<_size.y() ; ++j)
                get({i,j}) = dat[i*_size.y()+j];
    }

    template <class T>
    ImageAlgorithm<T>::ImageAlgorithm(uivec2 s) : _data(nullptr), _size(0,0)
    {
        build(s);
    }

    template <class T>
    ImageAlgorithm<T>::ImageAlgorithm(const ImageAlgorithm& img) : _data(nullptr), _size(0,0)
    {
        build(img._size);
        for(uint i=0 ; i<_size.x() ; ++i)
            for(uint j=0 ; j<_size.y() ; ++j)
                get({i,j}) = img.get({i,j});
    }

    template <class T>
    ImageAlgorithm<T>::ImageAlgorithm(ImageAlgorithm&& img)
    {
        _data = img._data;
        _size = img._size;
        img._data = nullptr;
        img._size = uivec2(0,0);
    }

    template <class T>
    ImageAlgorithm<T>& ImageAlgorithm<T>::operator=(const ImageAlgorithm& img)
    {
        if(this == &img)
            return *this;

        if(_size != img._size)
            build(img._size);

        for(uint i=0 ; i<_size.x() ; ++i)
            for(uint j=0 ; j<_size.y() ; ++j)
                get({i,j}) = img.get({i,j});
    }

    template <class T>
    ImageAlgorithm<T>& ImageAlgorithm<T>::operator=(ImageAlgorithm&& img)
    {
        clear();
        _data = img._data;
        _size = img._size;
        img._data = nullptr;
        img._size = uivec2(0,0);
        return *this;
    }

    template <class T>
    void ImageAlgorithm<T>::build(uivec2 s)
    {
        clear();
        _size = s;
        _data = new T[s.x()*s.y()];
    }

    template <class T>
    void ImageAlgorithm<T>::clear()
    {
        delete[] _data;
        _data=nullptr;
        _size = uivec2(0,0);
    }

    template <class T>
    const T& ImageAlgorithm<T>::safe_get(uivec2 s) const
    {
        if(check(s)) return _data[s.x()*_size.y()+s.y()];
        else return T();
    }

    template <class T>
    T& ImageAlgorithm<T>::safe_get(uivec2 s)
    {
        if(check(s)) return _data[s.x()*_size.y()+s.y()];
        else return _data[0];
    }

    template <class T>
    const T& ImageAlgorithm<T>::clamp_get(int x, int y) const
    {
        if(empty()) return get(uivec2());
        x = std::max(std::min(x, static_cast<int>(_size.x())-1), 0);
        y = std::max(std::min(y, static_cast<int>(_size.y())-1), 0);
        return get({ static_cast<uint>(x), static_cast<uint>(y) });
    }

    template <class T>
    T& ImageAlgorithm<T>::clamp_get(int x, int y)
    {
        if(empty()) get(uivec2());
        x = std::max(std::min(x, static_cast<int>(_size.x())-1), 0);
        y = std::max(std::min(y, static_cast<int>(_size.y())-1), 0);
        return get({ static_cast<uint>(x), static_cast<uint>(y) });
    }

    template <class T>
    const T& ImageAlgorithm<T>::get(uivec2 s) const
    {
        return _data[s.x()*_size.y()+s.y()];
    }

    template <class T>
    T& ImageAlgorithm<T>::get(uivec2 s)
    {
        return _data[s.x()*_size.y()+s.y()];
    }

    template <class T>
    void ImageAlgorithm<T>::set(uint x, uint y, const T& dat)
    {
        if(check({x,y}))
            _data[x*_size.y()+y] = dat;
    }

    template <class T>
    T* ImageAlgorithm<T>::detachData()
    {
        T* dat = _data;
        _data = nullptr;
        _size = uivec2(0,0);
        return dat;
    }

    template <class T>
    ImageAlgorithm<T> ImageAlgorithm<T>::blur3x3() const
    {
        ImageAlgorithm<T> img(_size);
        for(int i=0 ; i<static_cast<int>(_size.x()) ; ++i)
        {
            for(int j=0 ; j<static_cast<int>(_size.y()) ; ++j)
            {
                T c = clamp_get(i-1,j-1)*0.25 + clamp_get(i,j-1)*0.5 + clamp_get(i+1,j-1)*0.25
                    + clamp_get(i-1,j)*0.5 + clamp_get(i,j) + clamp_get(i+1,j)*0.5
                    + clamp_get(i-1,j+1)*0.25 + clamp_get(i,j+1)*0.5 + clamp_get(i+1,j+1)*0.25;

                img.set(i,j, c*0.25);
            }
        }
        return img;
    }

    template <class T>
    template<uint KS>
    ImageAlgorithm<T> ImageAlgorithm<T>::blur() const
    {
        static_assert(KS%2==1, "KS must be odd.");
        static PascaleTriangle COEF(KS);

        ImageAlgorithm<T> imgH(_size);
        for(int i=0 ; i<static_cast<int>(_size.x()) ; ++i)
            for(int j=0 ; j<static_cast<int>(_size.y()) ; ++j)
        {
            T c;
            for(int k=-(int(KS)-1)/2 ; k<=(int(KS)-1)/2 ; ++k)
                c += clamp_get(i+k,j) * float(float(COEF.getRow(KS-1)[k+(KS-1)/2]) / (1<<(KS-1)));

            imgH.set(i,j, c);
        }

        ImageAlgorithm<T> imgV(_size);
        for(int i=0 ; i<static_cast<int>(_size.x()) ; ++i)
            for(int j=0 ; j<static_cast<int>(_size.y()) ; ++j)
        {
            T c;
            for(int k=-(int(KS)-1)/2 ; k<=(int(KS)-1)/2 ; ++k)
                c += imgH.clamp_get(i,j+k) * float(float(COEF.getRow(KS-1)[k+(KS-1)/2]) / (1<<(KS-1)));

            imgV.set(i,j, c);
        }

        return imgV;
    }


}
}
#include "MemoryLoggerOff.h"

#endif // IMAGE_ALGORITHM
