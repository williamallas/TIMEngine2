#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

#include "Timath.h"
#include "StringUtils.h"
#include <type_traits>

#include "MemoryLoggerOn.h"

namespace tim
{
namespace core
{

    template <class T, size_t N>
    class Vector
    {
    public:
        Vector() { for(size_t i=0;i<N;++i)_val[i]=T(); }
        //Vector(const Vector& v) { for(size_t i=0;i<N;++i)_val[i]=v._val[i]; }
        //Vector(const T& val) { for(size_t i=0;i<N;++i)_val[i]=val; }
        //template<size_t A> explicit Vector(const Vector<T,A>& v) : Vector() { for(size_t i=0;i<std::min(N,A);++i)_val[i]=v[i]; }

        Vector(std::initializer_list<T> l)
        {
            auto it=l.begin();
            for(size_t i=0;it!=l.end() && i<N;++i)
            {
                _val[i]=*it;
                it++;
            }
        }

#ifndef USE_VCPP
        template <class First, class... Rest>
        explicit Vector(First first, Rest... rest)
        {
                for(size_t i=0;i<N;++i)
                    _val[i]=T();
                construct<0, First, Rest... >(first, rest... );
        }
#else
		explicit Vector(const T& x, const T& y) : Vector()
		{ static_assert(N >= 2, "To much data in constructor"); _val[0] = x; _val[1] = y; }
		explicit Vector(const T& x, const T& y, const T& z) : Vector()
		{ static_assert(N >= 3, "To much data in constructor"); _val[0] = x; _val[1] = y; _val[2] = z; }
		explicit Vector(const T& x, const T& y, const T& z, const T& w) : Vector()
		{ static_assert(N >= 4, "To much data in constructor"); _val[0] = x; _val[1] = y; _val[2] = z; _val[3] = 2; }

		explicit Vector(const Vector<T, N + 1>& v)
		{
			for (int i = 0; i < N; ++i)
				_val[i] = v[i];
		}

		explicit Vector(const Vector<T, N - 1>& v, const T& x)
		{
			for (int i = 0; i < N-1; ++i)
				_val[i] = v[i];
			_val[N - 1] = x;
		}

		explicit Vector(const Vector<T, N - 1>& v)
		{
			for (int i = 0; i < N - 1; ++i)
				_val[i] = v[i];
			_val[N - 1] = 0;
		}
#endif

        static Vector construct(const T& x)
        {
            Vector v;
            for(size_t i=0;i<N;++i)v._val[i]=x;
            return v;
        }

        const T& x() const { return _val[0]; }
        const T& y() const { static_assert(N>1, "Invalid component access."); return _val[1]; }
        const T& z() const { static_assert(N>2, "Invalid component access."); return _val[2]; }
        const T& w() const { static_assert(N>3, "Invalid component access."); return _val[3]; }

        T& x() { return _val[0]; }
        T& y() { static_assert(N>1, "Invalid component access."); return _val[1]; }
        T& z() { static_assert(N>2, "Invalid component access."); return _val[2]; }
        T& w() { static_assert(N>3, "Invalid component access."); return _val[3]; }

        Vector& set(const T& v, int i) { _val[i] = v; return *this; }

        Vector& operator=(const Vector& v) { for(size_t i=0;i<N;++i)_val[i]=v[i]; return *this; }
        bool operator==(const Vector& v) const { for(size_t i=0;i<N;++i){if(_val[i]!=v[i])return false;} return true; }
        bool operator!=(const Vector& v) const { return !((*this)==v); }

        Vector operator+(const Vector& v) const { Vector vec; for(size_t i=0;i<N;++i)vec.set(_val[i]+v[i], i); return vec; }
        Vector operator-(const Vector& v) const { Vector vec; for(size_t i=0;i<N;++i)vec.set(_val[i]-v[i], i); return vec; }
        Vector operator*(const Vector& v) const { Vector vec; for(size_t i=0;i<N;++i)vec.set(_val[i]*v[i], i); return vec; }
        Vector operator/(const Vector& v) const { Vector vec; for(size_t i=0;i<N;++i)vec.set(_val[i]/v[i], i); return vec; }

        Vector& operator+=(const Vector& v)  { for(size_t i=0;i<N;++i)_val[i]+=v[i]; return *this; }
        Vector& operator-=(const Vector& v)  { for(size_t i=0;i<N;++i)_val[i]-=v[i]; return *this; }
        Vector& operator*=(const Vector& v)  { for(size_t i=0;i<N;++i)_val[i]*=v[i]; return *this; }
        Vector& operator/=(const Vector& v)  { for(size_t i=0;i<N;++i)_val[i]/=v[i]; return *this; }

        Vector operator+(const T& v) const { Vector vec; for(size_t i=0;i<N;++i)vec.set(_val[i]+v, i); return vec; }
        Vector operator-(const T& v) const { Vector vec; for(size_t i=0;i<N;++i)vec.set(_val[i]-v, i); return vec; }
        Vector operator*(const T& v) const { Vector vec; for(size_t i=0;i<N;++i)vec.set(_val[i]*v, i); return vec; }
        Vector operator/(const T& v) const { Vector vec; for(size_t i=0;i<N;++i)vec.set(_val[i]/v, i); return vec; }

        Vector& operator+=(const T& v) { for(size_t i=0;i<N;++i)_val[i]+=v; return *this; }
        Vector& operator-=(const T& v) { for(size_t i=0;i<N;++i)_val[i]-=v; return *this; }
        Vector& operator*=(const T& v) { for(size_t i=0;i<N;++i)_val[i]*=v; return *this; }
        Vector& operator/=(const T& v) { for(size_t i=0;i<N;++i)_val[i]/=v; return *this; }

        Vector operator-() const { return *this * -1; }

        bool operator<(const Vector& v) const
        {
            for(size_t i=0;i<N;++i)
            {
                if(_val[i] < v._val[i])
                    return true;
                else if(_val[i] > v._val[i])
                    return false;
            }
            return false;
        }

        bool operator>(const Vector& v) const { return   (v < *this); }
        bool operator<=(const Vector& v) const { return !(v > *this); }
        bool operator>=(const Vector& v) const { return !(*this < v); }

        T& operator[](size_t i) { return _val[i]; }
        const T& operator[](size_t i) const { return _val[i]; }
        const T* data() const { return _val; }

        T dot(const Vector<T,N>& v) const
        {
             T res=0;
             for(size_t i=0;i<N;++i) res+=(_val[i]*v[i]);
             return res;
        }

        T length2() const { T res=0; for(size_t i=0;i<N;++i)res+=(_val[i]*_val[i]); return res; }
        T length() const { return sqrt(length2()); }

        Vector& normalize() {(*this)/=length(); return *this; }
        Vector normalized() const { return (*this)/length(); }

        Vector& resize(float f) { normalize()*=f; return *this; }
        Vector resized(float f) const { return normalized()*f; }

        Vector cross(const Vector& v) const
        {
            Vector<T, N> res;
            for(size_t i=0; i < N; ++i)
                res[i] = _val[(i+1) % N] * v[(i+2) % N] - _val[(i+2) % N] * v[(i+1) % N];
            return res;
        }

        Vector& saturate()
        {
            T m=_val[0];
            for(size_t i=1 ; i<N ; ++i) m = std::max(_val[i], m);
            return *this /= m;
        }

        Vector saturated() const
        {
            Vector v=*this;
            v.saturate();
            return v;
        }

        Vector mod(const Vector& v) const
        {
            static_assert(std::is_floating_point<T>::value || std::is_integral<T>::value,
                          "Operation mod() not available.");

            if(std::is_floating_point<T>::value)
            {
                Vector res = *this;
                res /= v;
                res.apply(floorf);
                res *= v;
                return res;
            }
            else
            {
                Vector res;
                for(size_t i=0 ; i<N ; ++i) res[i] = static_cast<T>(core::mod<int>(_val[i], v[i]));
                return res;
            }
        }

        template<size_t A>
        Vector<T,A> to() const
        {
            Vector<T,A> v;
            for(size_t i=0 ; i<std::min(A,N) ; ++i) v[i]=_val[i];
            return v;
        }

        template<size_t A> Vector<T,N-A> down() const { static_assert(N-A>0, "Invalid vector size."); return to<N-A>(); }
        template<size_t A> Vector<T,N+A> extend() const { return to<N+A>(); }

        template<size_t A> int32_t hash() const
        {
            static_assert((A <= N) && (A <= 4), "Invalid vector size.");
            int32_t res=0;
            for(size_t i=0;i<A;++i)
            {
                memcpy(reinterpret_cast<sbyte*>(&res)+i, &_val[i], 4/A);
            }
            return res+1734642733;
        }

        template <class F>
        Vector& apply(const F& f) { for(size_t i=0 ; i<N ; ++i) _val[i]=f(_val[i]); return *this; }

        /* String util */
        std::string str() const
        {
            std::string str="(";
            for(size_t i=0;i<N-1;++i)
                str+=StringUtils(_val[i]).str()+",";
            str += StringUtils(_val[N-1]).str()+")";
            return str;
        }
        friend std::ostream& operator<< (std::ostream& stream, const Vector& t) { stream << t.str(); return stream;}

    protected:
        T _val[N];

        /* Variadic constructor helper */
        template<class TT>
        struct TypeLength : public  std::integral_constant<size_t, 1> {};

        template<class TT, size_t NN>
        struct TypeLength<Vector<TT,NN>> : public  std::integral_constant<size_t, std::is_same<T,Vector<TT,NN>>::value ? 1:NN> {};

        void copy_data_at(size_t INDEX, T val)
        {
            _val[INDEX]=val;
        }

        template<class TT, size_t NN>
        void copy_data_at(size_t INDEX, const Vector<TT,NN>& v)
        {
            if(INDEX >= N) return;

            for(size_t i=0 ; i<std::min(N-INDEX, NN) ; ++i)
                _val[INDEX+i] = v[i];
        }

        template<class TT>
        void copy_data_at(size_t INDEX, TT val)
        {
            _val[INDEX]=val;
        }

//        template <size_t INDEX, class First>
//        void construct(First first)
//        {
//            static_assert(INDEX!=0 || TypeLength<First>::value > 1 || N==1, "Warning use of constructor");
////            if(INDEX == 0 && TypeLength<First>::value == 1)
////                for(size_t i=0 ; i<N ; ++i) _val[i]=first;
////            else
//            copy_data_at(INDEX, first);
//            //construct<INDEX+TypeLength<First>::value, Rest...>(rest...);
//        }

        template <size_t INDEX, class First, class... Rest>
        void construct(First first, Rest... rest)
        {
            copy_data_at(INDEX, first);
            construct<INDEX+TypeLength<First>::value, Rest...>(rest...);
        }

        template <size_t INDEX>
        void construct() {}// terminate recursion
    };

    template <class T> using Vector4 = Vector<T, 4>;
    template <class T> using Vector3 = Vector<T, 3>;
    template <class T> using Vector2 = Vector<T, 2>;
    template <class T> using Vector1 = Vector<T, 1>;

    typedef Vector4<float> vec4;
    typedef Vector4<int> ivec4;
    typedef Vector4<size_t> uivec4;
    typedef Vector4<double> dvec4;
    typedef Vector4<ubyte> ubvec4;
    typedef Vector4<bool> bvec4;

    typedef Vector3<float> vec3;
    typedef Vector3<int> ivec3;
    typedef Vector3<size_t> uivec3;
    typedef Vector3<double> dvec3;
    typedef Vector3<ubyte> ubvec3;

    typedef Vector2<float> vec2;
    typedef Vector2<int> ivec2;
    typedef Vector2<size_t> uivec2;
    typedef Vector2<double> dvec2;
    typedef Vector2<ubyte> ubvec2;

    typedef Vector<float,1> vec1;
    typedef Vector<int,1> ivec1;
    typedef Vector<size_t,1> uivec1;
    typedef Vector<double,1> dvec1;
    typedef Vector<ubyte,1> ubvec1;

    template <size_t N>
    Vector<float, N> toVec(const std::string& str)
    {
        std::string tmp;
        Vector<float, N> res;
        size_t i=0;
        for(size_t index=0 ; index<N ; ++index)
        {
            while(i<str.size() && str[i] != ',' && str[i] != ' ')
            {
                tmp += str[i];
                ++i;
            }

            ++i;
            res[index] = StringUtils(tmp).toFloat();
            tmp.clear();
        }

        return res;
    }

    template <size_t N>
    Vector<int, N> toiVec(const std::string& str)
    {
        std::string tmp;
        Vector<int, N> res;
        size_t i=0;
        for(size_t index=0 ; index<N ; ++index)
        {
            while(i<str.size() && str[i] != ',' && str[i] != ' ')
            {
                tmp += str[i];
                ++i;
            }

            ++i;
            res[index] = StringUtils(tmp).toInt();
            tmp.clear();
        }

        return res;
    }

    template <size_t N>
    Vector<std::string, N> toStrVec(const std::string& str)
    {
        Vector<std::string, N> res;
        size_t i=0;
        for(size_t index=0 ; index<N ; ++index)
        {
            while(i<str.size() && str[i] != ',' && str[i] != ' ')
            {
                res[index] += str[i];
                ++i;
            }

            ++i;
        }

        return res;
    }

    inline vec4 slerp_quaternion(const vec4& q1, const vec4& q2, float t)
    {
        vec4 q3;
		float dot = q1.dot(q2);

		if (dot < 0)
		{
			dot = -dot;
			q3 = -q2;
		}else q3 = q2;

		if (dot < 0.95f)
		{
			float angle = acosf(dot);
			return ((q1*sinf(angle*(1-t)) + q3*sinf(angle*t))/sinf(angle)).normalized();
		} else // if the angle is small, use linear interpolation
			return interpolate(q1,q3,t).normalized();
    }

}
}

#include "MemoryLoggerOff.h"


#endif // VECTOR_H_INCLUDED
