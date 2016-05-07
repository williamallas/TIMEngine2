#ifndef MATRIX_H_INCLUDED
#define MATRIX_H_INCLUDED

#include "Vector.h"

#include "MemoryLoggerOn.h"
namespace tim
{
namespace core
{

    template <class T, size_t N>
    class Matrix
    {
    public:

        Matrix() { for(size_t i=0;i<N*N;++i)_val[i]=0; }
        Matrix(const Matrix& m) { for(size_t i=0;i<N*N;++i)_val[i]=m._val[i]; }

        Matrix(const T data[N*N]) { for(size_t i=0;i<N*N;++i)_val[i]=data[i]; }

        Matrix(std::initializer_list<T> l) : Matrix()
        {
            if(l.size() > N*N) return;
            auto it=l.begin();
            for(size_t i=0;it!=l.end();++i)
            {
                _val[i]=*it;
                it++;
            }
        }

//        template<size_t A> Matrix(const Matrix<T,A>& m) : Matrix()
//        {
//            *this = IDENTITY();
//            for(size_t i=0;i<std::min(N,A);++i)for(size_t j=0;j<std::min(N,A);++j)
//                _val[i*N+j]=m.get(i*A+j);
//        }

        Matrix& operator=(const Matrix& m) { for(size_t i=0;i<N*N;++i)_val[i]=m.get(i); return *this; }
        bool operator==(const Matrix& m) { for(size_t i=0;i<N*N;++i){if(_val[i]!=m.get(i)) return false;} return true; }
        bool operator!=(const Matrix& m) { return !(*this==m); }

        Matrix operator+(const Matrix& m) { Matrix res; for(size_t i=0;i<N*N;++i)res.get(i)=_val[i]+m.get(i); return res; }
        Matrix operator-(const Matrix& m) { Matrix res; for(size_t i=0;i<N*N;++i)res.get(i)=_val[i]-m.get(i); return res; }

        Matrix operator*(const Matrix& m) const
        {
            Matrix res;
            for(size_t i=0 ; i<N ; ++i)
                for(size_t j=0 ; j<N ; ++j)
                    for(size_t k=0 ; k<N ; ++k)
            {
                res.get(i*N+j)+=_val[i*N+k]*m.get(k*N+j);
            }
            return res;
        }

        Vector<T,N> operator*(const Vector<T,N>& v) const
        {
            Vector<T,N> res;
            for(size_t i=0 ; i<N ; ++i)
                res[i]=(*this)[i].dot(v);
            return res;
        }

        template<size_t A>
        Vector<T,A> operator*(const Vector<T,A>& v) const
        {
            static_assert(A<N, "Vector size must be less or equal than matrix dimension");
            Vector<T,A> res;
            for(size_t i=0 ; i<A ; ++i)
            {
                for(size_t j=0 ; j<A ; ++j)
                    res[i]+=(*this)[i][j]*v[j];
            }
            for(size_t i=0 ; i<A ; ++i)
            {
                for(size_t j=A ; j<N ; ++j)
                    res[i]+=(*this)[i][j];
            }
            return res;
        }

        Matrix& operator+=(const Matrix& m) { for(size_t i=0;i<N*N;++i)_val[i]+=m.get(i); return *this; }
        Matrix& operator-=(const Matrix& m) { for(size_t i=0;i<N*N;++i)_val[i]-=m.get(i); return *this; }
        Matrix& operator*=(const Matrix& m) { *this = *this*m; return *this; }

        const Vector<T,N>& operator[](size_t i) const { return _mat[i]; }
        Vector<T,N>& operator[](size_t i) { return _mat[i]; }

        bool operator<(const Matrix& m) const
        {
            for(size_t i=0;i<N;++i)
            {
                if(_mat[i] < m._mat[i])
                    return true;
                else if(!(_mat[i] < m._mat[i] && _mat[i]==m._mat[i]))
                    return false;
            }
            return false;
        }

        const T& get(size_t i) const { return _val[i]; }
        T& get(size_t i) { return _val[i]; }
        const T* data() const { return _val; }

        Matrix& setRow(const Vector<T,N>& row, size_t row_id) { for(size_t i=0;i<N;++i) _val[i*N+row_id]=row[i]; }

        Matrix& transpose() { for(size_t i=0;i<N;++i)for(size_t j=0;j<i;++j)std::swap(_val[i*N+j], _val[j*N+i]); return *this; }
        Matrix transposed() const { Matrix m; for(size_t i=0;i<N;++i)for(size_t j=0;j<N;++j)m.get(i*N+j)=_val[j*N+i]; return m; }

        Matrix& scale(const Vector<T,N-1>& v) { *this = scaled(v); return *this; }
        Matrix scaled(const Vector<T,N-1>& v) const { return *this * Scale(v); }

        Vector<T,N-1> translation() const { Vector<T,N-1> res; for(size_t i=0;i<N-1;++i)res[i]=_val[i*N+N-1]; return res; }
        Matrix& translate(const Vector<T,N-1>& v) { for(size_t i=0;i<N-1;++i)_val[i*N+N-1]+=v[i]; return *this; }
        Matrix translated(const Vector<T,N-1>& v) const { Matrix m(*this); for(size_t i=0;i<N-1;++i)m.get(i*N+N-1)+=v[i]; return m; }
        Matrix& setTranslation(const Vector<T,N-1>& v) { for(size_t i=0;i<N-1;++i)_val[i*N+N-1]=v[i]; return *this; }

        T determinant() const
        {
            static_assert(N<=4, "The matrix is to bigger to compute the determinant.");

            T det=0;
            switch(N)
            {
                case 0: det=0; break;
                case 1: det = _val[0]; break;

                case 2: det = _val[0]*_val[3]-_val[1]*_val[2]; break;

                case 3: det = _val[0] * (_val[4] * _val[8] - _val[7] * _val[5])
                             - _val[1] * (_val[3] * _val[8] - _val[6] * _val[5])
                             + _val[2] * (_val[3] * _val[7] - _val[6] * _val[4]); break;

                case 4: det = _val[0]*(_val[11]*(_val[6]*_val[13]-_val[5]*_val[14])
                                        +_val[10]*(_val[5]*_val[15]-_val[7]*_val[13])
                                        +_val[9]*(_val[7]*_val[14]-_val[6]*_val[15]))
                              +_val[8]*(_val[1]*(_val[6]*_val[15]-_val[7]*_val[14])
                                        +_val[2]*(_val[7]*_val[13]-_val[5]*_val[15])
                                        +_val[3]*(_val[5]*_val[14]-_val[6]*_val[13]))
                              +_val[11]*(_val[1]*(_val[4]*_val[14]-_val[6]*_val[12])+
                                         _val[2]*(_val[5]*_val[12]-_val[4]*_val[13]))
                              +_val[1]*(_val[7]*_val[10]*_val[12]-_val[4]*_val[10]*_val[15])
                              +_val[9]*(_val[2]*(_val[4]*_val[15]-_val[7]*_val[12])+
                                        _val[3]*(_val[6]*_val[12]-_val[4]*_val[14]))
                              +_val[3]*(_val[4]*_val[10]*_val[13]-_val[5]*_val[10]*_val[12]); break;

                default: det = 1; break;
            }
            return det;
        }

        Matrix inverted() const
        {
            Matrix inv;
            T invDet = 1.0f/determinant();
            for(size_t i=0 ; i<N ; ++i)
                for(size_t j=0 ; j<N ; ++j)
                    inv.get(j*N+i) = sub({i,j}).determinant() * (1-(int(i+j)%2)*2) * invDet;
            return inv;
        }

        Matrix& invert() { *this = this->inverted(); return *this; }

        template<size_t A>
        Matrix<T,A> to() const
        {
            Matrix<T,A> m=Matrix<T,A>::IDENTITY();
            for(size_t i=0;i<std::min(A,N);++i)for(size_t j=0;j<std::min(A,N);++j)
                m.get(i*A+j)=_val[i*N+j];
            return m;
        }

        template<size_t A> Matrix<T,N+A> extend() const { return to<N+A>(); }
        template<size_t A> Matrix<T,N-A> down() const { static_assert(N-A>1, "Can't create matrix of size less than 1."); return to<N-A>(); }

        Matrix<T,N-1> sub(const Vector2<size_t>& v) const
        {
            Matrix<T,N-1> m;
            size_t c=0;
            for(size_t i=0;i<N;++i)
                for(size_t j=0;j<N;++j)
            {
                if(i!=v.x() && j!=v.y())
                {
                    m.get(c)=get(i*N+j);
                    c++;
                }
            }
            return m;
        }

        /* String util */
        std::string str() const
        {
            std::string str="(";
            for(size_t i=0;i<N-1;++i)
            {
                str+='(';
                for(size_t j=0 ; j<N-1; ++j)
                    str+=StringUtils(_mat[i][j]).str()+",";
                str += StringUtils(_mat[i][N-1]).str()+"),";
            }
            str += '(';
            for(size_t j=0 ; j<N-1; ++j)
                str+=StringUtils(_mat[N-1][j]).str()+",";
            str+=StringUtils(_mat[N-1][N-1]).str()+"))";

            return str;
        }
        friend std::ostream& operator<< (std::ostream& stream, const Matrix& t) { stream << t.str(); return stream;}

        /** Static methods **/

        static const Matrix& ZERO()
        {
            static const Matrix zero;
            return zero;
        }

        static const Matrix& IDENTITY()
        {
            T data[N*N]={0};
            for(size_t i=0 ; i<N ; ++i) data[i*N+i]=1;
            static const Matrix identity(data);
            return identity;
        }

        static Matrix Scale(const Vector<T,N-1>& v)
        {
            Matrix m(Matrix::IDENTITY());
            for(size_t i=0 ; i<N-1 ; ++i)
                m.get(i*N+i)=v[i];
            return m;
        }

        static Matrix Translation(const Vector<T,N-1>& v)
        {
            Matrix m(Matrix::IDENTITY());
            for(size_t i=0 ; i<N-1 ; ++i)
                m.get(i*N+N-1)=v[i];
            return m;
        }

    protected:
        union
        {
            Vector<T,N> _mat[N];
            T _val[N*N];
        };
    };

    template<class T>
    class Matrix2 : public Matrix<T,2>
    {
    public:

        Matrix2() : Matrix<T,2>() {}
        Matrix2(const Matrix<T,2>& m) : Matrix<T,2>(m) {}
        Matrix2(const T data[4]) : Matrix<T,2>(data) {}
        Matrix2(std::initializer_list<T> l) : Matrix<T,2>(l) {}

        static Matrix2 Rotation(const T& angle)
        {
            T cosA = cos(angle);
            T sinA = sin(angle);
            return Matrix2({cosA, -sinA, sinA, cosA});
        }

        static const Matrix2& ROT_90()  {  static Matrix2 m = Matrix2({0,-1,1,0}); return m; }
        static const Matrix2& ROT_180() {  static Matrix2 m = Matrix2({-1,0,0,-1}); return m; }
        static const Matrix2& ROT_270() {  static Matrix2 m = Matrix2({0,1,-1,0}); return m; }
        static const Matrix2& FLIP_X()  {  static Matrix2 m = Matrix2({1,0,0,-1}); return m; }
        static const Matrix2& FLIP_Y()  {  static Matrix2 m = Matrix2({-1,0,0,1}); return m; }
        static const Matrix2& TRANSPOSE() {  static Matrix2 m = Matrix2({0,1,1,0}); return m; }
    };

    template<class T>
    class Matrix3 : public Matrix<T,3>
    {
    public:

        Matrix3() : Matrix<T,3>() {}
        Matrix3(const Matrix<T,3>& m) : Matrix<T,3>(m) {}
        Matrix3(const T data[9]) : Matrix<T,3>(data) {}
        Matrix3(std::initializer_list<T> l) : Matrix<T,3>(l) {}

        static Matrix3 RotationZ(T angle)
        {
            T cosA=cos(angle);
            T sinA=sin(angle);
            Matrix3 m=Matrix<T,3>::IDENTITY();

            m.get(0)=cosA; m.get(1)=-sinA;
            m.get(3)=sinA; m.get(4)=cosA;
            return m;
        }

        static Matrix3 RotationY(T angle)
        {
            T cosA=cos(angle);
            T sinA=sin(angle);
            Matrix3 m=Matrix<T,3>::IDENTITY();

            m.get(0)=cosA; m.get(2)=sinA;
            m.get(6)=-sinA; m.get(8)=cosA;
            return m;
        }

        static Matrix3 RotationX(T angle)
        {
            T cosA=cos(angle);
            T sinA=sin(angle);
            Matrix3 m=Matrix<T,3>::IDENTITY();

            m.get(4)=cosA; m.get(7)=sinA;
            m.get(5)=-sinA; m.get(8)=cosA;
            return m;
        }

        static Matrix3 AxisRotation(const Vector3<T>& axis, const Vector3<T>& daxis, T angle)
        {
            Matrix3 m;
            m[0] = axis.normalized();
            m[1] = axis.cross(daxis).normalized();
            m[2] = m[0].cross(m[1]);

            return m.inverted() * RotationX(angle) * m;
        }
    };

    template<class T>
    class Matrix4 : public Matrix<T,4>
    {
    public:
        Matrix4() : Matrix<T,4>() {}
        Matrix4(const Matrix<T,4>& m) : Matrix<T,4>(m) {}
        Matrix4(const T data[16]) : Matrix<T,4>(data) {}
        Matrix4(std::initializer_list<T> l) : Matrix<T,4>(l) {}

        static Matrix4 RotationZ(T angle)
        {
            T cosA=cos(angle);
            T sinA=sin(angle);
            Matrix4 m=Matrix<T,4>::IDENTITY();

            m.get(0)=cosA; m.get(1)=-sinA;
            m.get(4)=sinA; m.get(5)=cosA;
            return m;
        }

        static Matrix4 RotationY(T angle)
        {
            T cosA=cos(angle);
            T sinA=sin(angle);
            Matrix4 m=Matrix<T,4>::IDENTITY();

            m.get(0)=cosA; m.get(2)=sinA;
            m.get(8)=-sinA; m.get(10)=cosA;
            return m;
        }

        static Matrix4 RotationX(T angle)
        {
            T cosA=cos(angle);
            T sinA=sin(angle);
            Matrix4 m=Matrix<T,4>::IDENTITY();

            m.get(5)=cosA; m.get(9)=sinA;
            m.get(6)=-sinA; m.get(10)=cosA;
            return m;
        }

        static const Matrix4& BIAS()
        {
            static const Matrix4 m = { 0.5,0,0,0.5,
                                       0,0.5,0,0.5,
                                       0,0,0.5,0.5,
                                       0,0,0,1 };
            return m;
        }

        static const Matrix4& InvBIAS()
        {
            static const Matrix4 m = BIAS().inversed();
            return m;
        }

        static Matrix4 Projection(T fov, T ratio, T znear, T zfar)
        {
            fov /= ratio;

            T e = T(1) / tanf(toRad(fov)*T(0.5));
            Matrix4 m;
            m.get(0) = e/ratio;
            m.get(5) = e;
            m.get(10) = (znear+zfar)/(znear-zfar);
            m.get(11) = T(2)*zfar*znear/(znear-zfar);
            m.get(14)= -T(1);
            return m;
        }

        static Matrix4 Ortho(T l, T r, T b, T t, T n, T f)
        {
            T inv_r_l = T(1) / (r-l);
            T inv_t_b = T(1) / (t-b);
            T inv_f_n = T(1) / (f-n);

            Matrix4 m;
            m.get(0) = T(2) * inv_r_l;
            m.get(5) = T(2) * inv_t_b;
            m.get(10) = -T(2) * inv_f_n;
            m.get(15) = T(1);

            m.setTranslation({-(r+l)*inv_r_l, -(t+b)*inv_t_b, -(f+n)*inv_f_n});

            return m;
        }

        static Matrix4 View(const Vector3<T>& pos, const Vector3<T>& posView, const Vector3<T>& up)
        {
            Matrix4 m;
            Vector3<T> forward = (posView-pos).normalized();
            m[0] = Vector4<T>(forward.cross(up).normalized());
            m[1] = Vector4<T>(Vector3<T>(m[0]).cross(forward));
            m[2] = Vector4<T>(-forward);
            m[3] = {0,0,0,1};

            return m * Matrix4::Translation(-pos);
        }

        static Matrix4 AxisRotation(const Vector3<T>& axis, const Vector3<T>& daxis, T angle)
        {
            return Matrix4(Matrix3<T>::AxisRotation(axis, daxis, angle));
        }

        static Matrix4 convertQuaternion(const Vector4<T>& q)
        {
            return Matrix4({1-2*q.y()*q.y()-2*q.z()*q.z(), 2*q.x()*q.y()-2*q.z()*q.w(), 2*q.x()*q.z()+2*q.y()*q.w(), 0,
                            2*q.x()*q.y()+2*q.z()*q.w(), 1-2*q.x()*q.x()-2*q.z()*q.z(), 2*q.y()*q.z()-2*q.x()*q.w(), 0,
                            2*q.x()*q.z()-2*q.y()*q.w(), 2*q.y()*q.z()+2*q.x()*q.w(), 1-2*q.x()*q.x()-2*q.y()*q.y(), 0,
                            0,0,0,1});
        }
    };

    typedef Matrix4<float> mat4;
    typedef Matrix3<float> mat3;
    typedef Matrix2<float> mat2;

    typedef Matrix4<double> dmat4;
    typedef Matrix3<double> dmat3;
    typedef Matrix2<double> dmat2;

    typedef Matrix4<int> imat4;
    typedef Matrix3<int> imat3;
    typedef Matrix2<int> imat2;

    typedef Matrix4<size_t> uimat4;
    typedef Matrix3<size_t> uimat3;
    typedef Matrix2<size_t> uimat2;

}
}
#include "MemoryLoggerOff.h"

#endif // MATRIX_H_INCLUDED
