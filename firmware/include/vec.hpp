#pragma once

#include <array>
#include <cmath>

namespace lumina
{

template<int N, typename T>
class vec : public:: std::array<T, N>
{
public:

    T& x() requires(N >= 1) { return (*this)[0]; }
    T& y() requires(N >= 2) { return (*this)[1]; }
    T& z() requires(N >= 3) { return (*this)[2]; }

    T x() const requires(N >= 1) { return (*this)[0]; }
    T y() const requires(N >= 2) { return (*this)[1]; }
    T z() const requires(N >= 3) { return (*this)[2]; }

    vec normalized() const
    {
        T len = length();
        if(len == 0) return *this;
        return *this / len;
    }

    T length() const
    {
        return std::sqrt(dot(*this));
    }

    T dot(const vec &v) const
    {
        T sum = 0;
        for(int i = 0; i < N; i++)
            sum += (*this)[i] * v[i];
        return sum;
    }
    
    vec cross(const vec &v) const requires(N == 3)
    {
        return {y() * v.z() - z() * v.y(), z() * v.x() - x() * v.z(), x() * v.y() - y() * v.x()};
    }

    static T angle(const vec &a, const vec &b)
    {
        return std::acos(a.dot(b) / (a.length() * b.length()));
    }

    static T distance(const vec &a, const vec &b)
    {
        return (a - b).length();
    }

    vec operator+(const vec &v) const
    {
        vec result;
        for(int i = 0; i < N; i++)
            result[i] = (*this)[i] + v[i];
        return result;
    }

    vec operator-(const vec &v) const
    {
        vec result;
        for(int i = 0; i < N; i++)
            result[i] = (*this)[i] - v[i];
        return result;
    }

    vec operator*(T s) const
    {
        vec result;
        for(int i = 0; i < N; i++)
            result[i] = (*this)[i] * s;
        return result;
    }

    vec operator/(T s) const
    {
        vec result;
        for(int i = 0; i < N; i++)
            result[i] = (*this)[i] / s;
        return result;
    }

    vec &operator+=(const vec &v)
    {
        return *this = *this + v;
    }

    vec &operator-=(const vec &v)
    {
        return *this = *this - v;
    }

    vec &operator*=(T s)
    {
        return *this = *this * s;
    }

    vec &operator/=(T s)
    {
        return *this = *this / s;
    }

};

using vec2 = vec<2, float>;
using vec3 = vec<3, float>;
using vec4 = vec<4, float>;

}
