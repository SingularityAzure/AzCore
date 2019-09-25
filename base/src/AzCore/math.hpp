/*
    File: math.hpp
    Author: Philip Haynes
    Description: Common math routines and data types.
    Notes:
        - Vector math is right-handed.
        - Be aware of memory alignment when dealing with GPU memory.
*/
#ifndef MATH_HPP
#define MATH_HPP

// Some quick defines to change what gets implemented in math.cpp
// Comment these out to reduce compile size.
#define MATH_VEC2
#define MATH_VEC3
#define MATH_VEC4
#define MATH_VEC5
#define MATH_MAT2
#define MATH_MAT3
#define MATH_MAT4
#define MATH_MAT5
#define MATH_COMPLEX
#define MATH_QUATERNION
// MATH_EQUATIONS adds solvers for linear, quadratic, and cubic equations
#define MATH_EQUATIONS
#define MATH_F32
// #define MATH_F64

// Some are dependent on others
#if defined(MATH_MAT2) && !defined(MATH_VEC2)
    #define MATH_VEC2
#endif
#if defined(MATH_MAT3) && !defined(MATH_VEC3)
        #define MATH_VEC3
#endif
#if defined(MATH_MAT4) && !defined(MATH_VEC3)
    #define MATH_VEC3
#endif
#if defined(MATH_MAT4) && !defined(MATH_VEC4)
    #define MATH_VEC4
#endif
#if defined(MATH_QUATERNION) && !defined(MATH_VEC3)
    #define MATH_VEC3
#endif

#if !defined(MATH_F32) && !defined(MATH_F64)
    #error "math.hpp needs to have one or both of MATH_F32 and MATH_F64 defined"
#endif

#include "basictypes.hpp"

#include <chrono>

using Nanoseconds = std::chrono::nanoseconds;
using Milliseconds = std::chrono::milliseconds;
using Clock = std::chrono::steady_clock;
using ClockTime = std::chrono::steady_clock::time_point;

#include <math.h>

const f64 halfpi64  = 1.5707963267948966;
const f64 pi64      = 3.1415926535897932;
const f64 tau64     = 6.2831853071795865;

const f32 halfpi    = (f32)halfpi64;
const f32 pi        = (f32)pi64;
const f32 tau       = (f32)tau64;

enum Axis {
    X=0, Y=1, Z=2
};

enum Plane {
    XY=0,   XZ=1,   XW=2,
    YX=XY,  YZ=3,   YW=4,
    ZX=XZ,  ZY=YZ,  ZW=5
};

// #ifdef MATH_F32
//     inline f32 sin(f32 a) { return sinf(a); }
//     inline f32 cos(f32 a) { return cosf(a); }
//     inline f32 tan(f32 a) { return tanf(a); }
//     inline f32 asin(f32 a) { return asinf(a); }
//     inline f32 acos(f32 a) { return acosf(a); }
//     inline f32 atan(f32 a) { return atanf(a); }
//     inline f32 atan2(f32 y, f32 x) { return atan2f(y, x); }
//     inline f32 sqrt(f32 a) { return sqrtf(a); }
//     inline f32 exp(f32 a) { return expf(a); }
//     inline f32 log(f32 a) { return logf(a); }
//     inline f32 log2(f32 a) { return log2f(a); }
//     inline f32 log10(f32 a) { return log10f(a); }
//     inline f32 pow(f32 a, f32 b) { return powf(a, b); }
//     inline f32 floor(f32 a) { return floorf(a); }
//     inline f32 round(f32 a) { return roundf(a); }
//     inline f32 ceil(f32 a) { return ceilf(a); }
// #endif

/*  struct: RandomNumberGenerator
    Author: Philip Haynes
    Uses the JKISS generator by David Jones
    From http://www0.cs.ucl.ac.uk/staff/d.jones/GoodPracticeRNG.pdf */
struct RandomNumberGenerator {
    u32 x, y, z, c;
    RandomNumberGenerator(); // Automatically seeds itself based on time.
    u32 Generate();
    void Seed(u64 seed);
};

f32 random(f32 min, f32 max, RandomNumberGenerator& rng);

template<typename T>
inline T square(const T &a) {
    return a*a;
}

template<typename T>
inline T median(const T &a, const T &b, const T &c) {
    if ((b >= a && a >= c)||(c >= a && a >= b))
        return a;
    if ((a >= b && b >= c)||(c >= b && b >= a))
        return b;
    if ((a >= c && c >= b)||(b >= c && c >= a))
        return c;
    return T();
}

template<typename T>
inline T min(const T &a, const T &b) {
    return a > b ? b : a;
}

template<typename T>
inline T max(const T &a, const T &b) {
    return a > b ? a : b;
}

template<typename T>
inline T clamp(const T &a, const T &b, const T &c) {
    return median(a, b, c);
}

template<typename T>
inline T abs(const T &a) {
    return a >= 0 ? a : -a;
}

template<typename T>
inline T sign(const T &a) {
    return a >= 0 ? 1 : -1;
}

template<typename T, typename F>
inline T lerp(const T &a, const T &b, F factor) {
    factor = clamp(factor, F(0.0), F(1.0));
    return a + (b-a) * factor;
}

template<typename T, typename F>
T decay(T a, T b, F halfLife, F timestep) {
    F fac = exp(-timestep/halfLife);
    if (fac > F(1.0))
        fac = F(1.0);
    else if (fac < F(0.0))
        fac = F(0.0);
    return b * (F(1.0) - fac) + a * fac;
}

template<typename T>
inline T map(const T &in, const T &minFrom, const T &maxFrom, const T &minTo, const T &maxTo) {
    return (in - minFrom) * (maxTo - minTo) / (maxFrom - minFrom) + minTo;
}

template<typename T>
inline T cubert(const T &a) {
    return a >= T(0.0) ? pow(a, T(1.0/3.0)) : -pow(-a, T(1.0/3.0));
}

template<typename T>
inline T wrap(T a, T max) {
    while (a > max) {
        a -= max;
    }
    while (a < 0) {
        a += max;
    }
    return a;
}

template<typename T>
class Radians;
template<typename T>
class Angle;

/*  class: Degrees
    Author: Philip Haynes
    A discrete type that represents an angle in degrees.    */
template<typename T>
class Degrees {
    T _value;
public:
    Degrees() = default;
    Degrees(T a) : _value(a) {}
    Degrees(const Degrees<T>& a) : _value(a._value) {}
    Degrees(const Radians<T>& a) : _value(a._value*tau64/360.0d) {}
    Degrees<T>& operator+=(const Degrees<T>& other) { _value += other._value; return *this; }
    Degrees<T>& operator-=(const Degrees<T>& other) { _value -= other._value; return *this; }
    Degrees<T>& operator*=(const Degrees<T>& other) { _value *= other._value; return *this; }
    Degrees<T>& operator/=(const Degrees<T>& other) { _value /= other._value; return *this; }
    Degrees<T> operator+(const Degrees<T>& other) const { return Degrees<T>(_value+other._value); }
    Degrees<T> operator-(const Degrees<T>& other) const { return Degrees<T>(_value-other._value); }
    Degrees<T> operator*(const Degrees<T>& other) const { return Degrees<T>(_value*other._value); }
    Degrees<T> operator/(const Degrees<T>& other) const { return Degrees<T>(_value/other._value); }
    bool operator==(const Degrees<T>& other) const { return _value == other._value; }
    bool operator!=(const Degrees<T>& other) const { return _value != other._value; }
    bool operator<=(const Degrees<T>& other) const { return _value <= other._value; }
    bool operator>=(const Degrees<T>& other) const { return _value >= other._value; }
    bool operator<(const Degrees<T>& other) const { return _value < other._value; }
    bool operator>(const Degrees<T>& other) const { return _value > other._value; }
    const T value() const { return _value; }
};

/*  class: Radians
    Author: Philip Haynes
    A discrete type that represents an angle in radians.    */
template<typename T>
class Radians {
    T _value;
public:
    Radians() = default;
    Radians(T a) : _value(a) {}
    Radians(const Radians<T>& a) : _value(a._value) {}
    Radians(const Angle<T>& a) : _value(a.value()) {}
    Radians(const Degrees<T>& a) {
        if constexpr (std::is_same<T, f32>()) {
            _value = a.value()*tau/360.0;
        } else {
            _value = a.value()*tau64/360.0d;
        }
    }
    // Radians<T>& operator=(const Radians<T>& other) {
    //     _value = other._value;
    //     return *this;
    // }
    Radians<T>& operator+=(const Radians<T>& other) { _value += other._value; return *this; }
    Radians<T>& operator-=(const Radians<T>& other) { _value -= other._value; return *this; }
    Radians<T>& operator*=(const Radians<T>& other) { _value *= other._value; return *this; }
    Radians<T>& operator/=(const Radians<T>& other) { _value /= other._value; return *this; }
    Radians<T> operator+(const Radians<T>& other) const { return Radians<T>(_value+other._value); }
    Radians<T> operator-(const Radians<T>& other) const { return Radians<T>(_value-other._value); }
    Radians<T> operator*(const Radians<T>& other) const { return Radians<T>(_value*other._value); }
    Radians<T> operator/(const Radians<T>& other) const { return Radians<T>(_value/other._value); }
    bool operator==(const Radians<T>& other) const { return _value == other._value; }
    bool operator!=(const Radians<T>& other) const { return _value != other._value; }
    bool operator<=(const Radians<T>& other) const { return _value <= other._value; }
    bool operator>=(const Radians<T>& other) const { return _value >= other._value; }
    bool operator<(const Radians<T>& other) const { return _value < other._value; }
    bool operator>(const Radians<T>& other) const { return _value > other._value; }
    const T value() const { return _value; }
};

template<typename T>
inline T sin(const Radians<T>& a) { return sin(a.value()); }
template<typename T>
inline T cos(const Radians<T>& a) { return cos(a.value()); }
template<typename T>
inline T tan(const Radians<T>& a) { return tan(a.value()); }

/*  class: Angle
    Author: Philip Haynes
    A discrete type to represent all angles while regarding the circular nature of angles.   */
template<typename T>
class Angle {
    Radians<T> _value;
public:
    Angle() = default;
    Angle(const Angle<T>& other) : _value(other._value) {}
    Angle(const T& other) : _value(Radians<T>(other)) {}
    Angle(const Degrees<T>& other) : _value(Radians<T>(other)) {}
    Angle(const Radians<T>& other) {
        _value = other;
        if constexpr (std::is_same<T, f32>()) {
            while (_value > tau) {
                _value -= tau;
            }
            while (_value < 0.0) {
                _value += tau;
            }
        } else {
            while (_value > tau64) {
                _value -= tau64;
            }
            while (_value < 0.0d) {
                _value += tau64;
            }
        }
    }
    Angle<T>& operator+=(const Radians<T>& other) { return *this = _value + other; }
    Angle<T> operator+(const Radians<T>& other) const { return Angle<T>(_value+other); }
    Radians<T> operator-(const Angle<T>& other) const;
    bool operator==(const Angle<T>& other) const { return _value == other._value; }
    bool operator!=(const Angle<T>& other) const { return _value != other._value; }
    const T value() const { return _value.value(); }
};

template<typename T>
inline T sin(const Angle<T>& a) { return sin(a.value()); }
template<typename T>
inline T cos(const Angle<T>& a) { return cos(a.value()); }
template<typename T>
inline T tan(const Angle<T>& a) { return tan(a.value()); }

// Finds the shortest distance from one angle to another.
#ifdef MATH_F32
    typedef Degrees<f32> Degrees32;
    typedef Radians<f32> Radians32;
    typedef Angle<f32> Angle32;
    Radians32 angleDiff(Angle32 from, Angle32 to);
#endif
#ifdef MATH_F64
    typedef Degrees<f64> Degrees64;
    typedef Radians<f64> Radians64;
    typedef Angle<f64> Angle64;
    Radians64 angleDiff(Angle64 from, Angle64 to);
#endif

template<typename T>
Radians<T> Angle<T>::operator-(const Angle<T>& to) const {
    return angleDiff(*this, to);
}

template<typename T>
inline Radians<T> angleDir(const Angle<T>& from, const Angle<T>& to) {
    return sign(angleDiff(from, to));
}

template<typename T>
inline T normalize(const T& a) {
    return a / abs(a);
}

#ifdef MATH_VEC2
    template<typename T>
    struct vec2_t {
        union {
            struct {
                T x, y;
            };
            struct {
                T u, v;
            };
            struct {
                T data[2];
            };
        };

        vec2_t() = default;
        inline vec2_t(const T &a) : x(a), y(a) {};
        inline vec2_t(const T &a, const T &b) : x(a), y(b) {};
        template<typename I>
        vec2_t(const vec2_t<I> &a) : x((T)a.x) , y((T)a.y) {}
        inline vec2_t<T> operator+(const vec2_t<T>& a) const { return vec2_t<T>(x+a.x, y+a.y); }
        inline vec2_t<T> operator-(const vec2_t<T>& a) const { return vec2_t<T>(x-a.x, y-a.y); }
        inline vec2_t<T> operator-() const { return vec2_t<T>(-x, -y); }
        inline vec2_t<T> operator*(const vec2_t<T>& a) const { return vec2_t<T>(x*a.x, y*a.y); }
        inline vec2_t<T> operator/(const vec2_t<T>& a) const { return vec2_t<T>(x/a.x, y/a.y); }
        inline vec2_t<T> operator*(const T& a) const { return vec2_t<T>(x*a, y*a); }
        inline vec2_t<T> operator/(const T& a) const { return vec2_t<T>(x/a, y/a); }
        inline bool operator==(const vec2_t<T>& a) const { return x == a.x && y == a.y; }
        inline bool operator!=(const vec2_t<T>& a) const { return x != a.x || y != a.y; }
        inline T& operator[](const u32& i) { return data[i]; }
        vec2_t<T> operator+=(const vec2_t<T>& a);
        vec2_t<T> operator-=(const vec2_t<T>& a);
        vec2_t<T> operator/=(const vec2_t<T>& a);
        vec2_t<T> operator/=(const T& a);
        vec2_t<T> operator*=(const vec2_t<T>& a);
        vec2_t<T> operator*=(const T& a);
    };

    template<typename T>
    inline vec2_t<T> operator*(const T& a, const vec2_t<T>& b) {
        return b * a;
    }

    template<typename T>
    inline T dot(const vec2_t<T>& a, const vec2_t<T>& b) {
        return a.x*b.x + a.y*b.y;
    }

    template<typename T>
    inline T absSqr(const vec2_t<T>& a) {
        return a.x*a.x + a.y*a.y;
    }

    template<typename T>
    inline T abs(const vec2_t<T>& a) {
        return sqrt(a.x*a.x + a.y*a.y);
    }

    template<bool isSegment, typename T>
    inline T distSqrToLine(const vec2_t<T> &segA, const vec2_t<T> &segB, const vec2_t<T> &point) {
        const vec2_t<T> diff = segA - segB;
        const T lengthSquared = absSqr(diff);
        const T t = dot(diff, segA - point) / lengthSquared;
        vec2_t<T> projection;
        if constexpr (isSegment) {
            if (t < 0.0) {
                projection = segA;
            } else if (t > 1.0) {
                projection = segB;
            } else {
                projection = segA - diff * t;
            }
        } else {
            projection = segA - diff * t;
        }
        return absSqr(point - projection);
    }
#endif // MATH_VEC2

#ifdef MATH_MAT2
    template<typename T>
    struct mat2_t {
        union {
            struct {
                T x1, y1,
                  x2, y2;
            } h;
            struct {
                T x1, x2,
                  y1, y2;
            } v;
            struct {
                T data[4];
            };
        };
        mat2_t() = default;
        inline mat2_t(const T &a) : h{a, 0, 0, a} {};
        inline mat2_t(const T &a, const T &b, const T &c, const T &d) : h{a, b, c, d} {};
        template<bool rowMajor = true>
        inline mat2_t(const vec2_t<T> &a, const vec2_t<T> &b) {
            if constexpr (rowMajor) {
                h = {a.x, a.y, b.x, b.y};
            } else {
                h = {a.x, b.x, a.y, b.y};
            }
        }
        inline mat2_t(const T d[4]) : data{d[0], d[1], d[2], d[3]} {};
        inline vec2_t<T> Row1() const { return vec2_t<T>(h.x1, h.y1); }
        inline vec2_t<T> Row2() const { return vec2_t<T>(h.x2, h.y2); }
        inline vec2_t<T> Col1() const { return vec2_t<T>(v.x1, v.y1); }
        inline vec2_t<T> Col2() const { return vec2_t<T>(v.x2, v.y2); }
        inline static mat2_t<T> Identity() {
            return mat2_t(1);
        };
        static mat2_t<T> Rotation(T angle);
        static mat2_t<T> Skewer(vec2_t<T> amount);
        static mat2_t<T> Scaler(vec2_t<T> scale);
        inline mat2_t<T> Transpose() const {
            return mat2_t<T>(v.x1, v.y1, v.x2, v.y2);
        }
        inline mat2_t<T> operator+(const mat2_t<T>& a) const {
            return mat2_t<T>(
                h.x1 + a.h.x1, h.y1 + a.h.y1,
                h.x2 + a.h.x2, h.y2 + a.h.y2
            );
        }
        inline mat2_t<T> operator*(const mat2_t<T>& a) const {
            return mat2_t<T>(
                h.x1 * a.v.x1 + h.y1 * a.v.y1, h.x1 * a.v.x2 + h.y1 * a.v.y2,
                h.x2 * a.v.x1 + h.y2 * a.v.y1, h.x2 * a.v.x2 + h.y2 * a.v.y2
            );
        }
        inline vec2_t<T> operator*(const vec2_t<T>& a) const {
            return vec2_t<T>(
                h.x1*a.x + h.y1*a.y,
                h.x2*a.x + h.y2*a.y
            );
        }
        inline mat2_t<T> operator*(const T& a) const {
            return mat2_t<T>(
                h.x1*a, h.y1*a,
                h.x2*a, h.y2*a
            );
        }
    };

    template<typename T>
    inline vec2_t<T> operator*(const vec2_t<T> &a, const mat2_t<T> &b) {
        return vec2_t<T>(
            a.x*b.v.x1 + a.y*b.v.y1,
            a.x*b.v.x2 + a.y*b.v.y2
        );
    }
#endif // MATH_MAT2

#ifdef MATH_VEC3
    template<typename T>
    struct vec3_t {
        union {
            struct {
                T x, y, z;
            };
            struct {
                T r, g, b;
            };
            struct {
                T h, s, v;
            };
            struct {
                T data[3];
            };
        };

        vec3_t() = default;
        inline vec3_t(const T &a) : x(a) , y (a) , z(a) {}
        inline vec3_t(const T &v1, const T &v2, const T &v3) : x(v1) , y(v2) , z(v3) {}
        template<typename I>
        inline vec3_t(const vec3_t<I> &a) : x((T)a.x) , y((T)a.y), z((T)a.z) {}
        inline vec3_t<T> operator+(const vec3_t<T>& a) const { return vec3_t<T>(x+a.x, y+a.y, z+a.z); }
        inline vec3_t<T> operator-(const vec3_t<T>& a) const { return vec3_t<T>(x-a.x, y-a.y, z-a.z); }
        inline vec3_t<T> operator-() const { return vec3_t<T>(-x, -y, -z); }
        inline vec3_t<T> operator*(const vec3_t<T>& a) const { return vec3_t<T>(x*a.x, y*a.y, z*a.z); }
        inline vec3_t<T> operator/(const vec3_t<T>& a) const { return vec3_t<T>(x/a.x, y/a.y, z/a.z); }
        inline vec3_t<T> operator*(const T& a) const { return vec3_t<T>(x*a, y*a, z*a); }
        inline vec3_t<T> operator/(const T& a) const { return vec3_t<T>(x/a, y/a, z/a); }
        inline bool operator==(const vec3_t<T>& a) const { return x == a.x && y == a.y && z == a.z; }
        inline bool operator!=(const vec3_t<T>& a) const { return x != a.x || y != a.y || z != a.z; }
        inline T& operator[](const u32& i) { return data[i]; }
        vec3_t<T> operator+=(const vec3_t<T>& a);
        vec3_t<T> operator-=(const vec3_t<T>& a);
        vec3_t<T> operator/=(const vec3_t<T>& a);
        vec3_t<T> operator/=(const T& a);
        vec3_t<T> operator*=(const vec3_t<T>& a);
        vec3_t<T> operator*=(const T& a);
    };

    template<typename T>
    inline vec3_t<T> operator*(const T& a, const vec3_t<T>& b) {
        return b * a;
    }

    template<typename T>
    inline vec3_t<T> cross(const vec3_t<T>& a, const vec3_t<T>& b) {
        return vec3_t<T>(
            a.y*b.z - a.z*b.y,
            a.z*b.x - a.x*b.z,
            a.x*b.y - a.y*b.x
        );
    }

    template<typename T>
    inline T dot(const vec3_t<T>& a, const vec3_t<T>& b) {
        return a.x*b.x + a.y*b.y + a.z*b.z;
    }

    template<typename T>
    inline T absSqr(const vec3_t<T>& a) {
        return a.x*a.x + a.y*a.y + a.z*a.z;
    }

    template<typename T>
    inline T abs(const vec3_t<T>& a) {
        return sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
    }

    template<bool isSegment, typename T>
    T distSqrToLine(const vec3_t<T> &segA, const vec3_t<T> &segB, const vec3_t<T> &point) {
        const vec3_t<T> diff = segA - segB;
        const T lengthSquared = absSqr(diff);
        const T t = dot(diff, segA - point) / lengthSquared;
        vec3_t<T> projection;
        if constexpr (isSegment) {
            if (t < 0.0) {
                projection = segA;
            } else if (t > 1.0) {
                projection = segB;
            } else {
                projection = segA - diff * t;
            }
        } else {
            projection = segA - diff * t;
        }
        return absSqr(point - projection);
    }

    template<typename T>
    vec3_t<T> hsvToRgb(vec3_t<T> hsv);

    template<typename T>
    vec3_t<T> rgbToHsv(vec3_t<T> rgb);

#endif // MATH_VEC3

#ifdef MATH_MAT3
    template<typename T>
    struct mat3_t {
        union {
            struct {
                T x1, y1, z1,
                  x2, y2, z2,
                  x3, y3, z3;
            } h;
            struct {
                T x1, x2, x3,
                  y1, y2, y3,
                  z1, z2, z3;
            } v;
            struct {
                T data[9];
            };
        };
        mat3_t() = default;
        inline mat3_t(const T &a) : h{a, 0, 0, 0, a, 0, 0, 0, a} {}
        inline mat3_t(const T &x1, const T &y1, const T &z1,
                      const T &x2, const T &y2, const T &z2,
                      const T &x3, const T &y3, const T &z3) :
                data{x1, y1, z1, x2, y2, z2, x3, y3, z3} {}
        template<bool rowMajor = true>
        inline mat3_t(const vec3_t<T> &a, const vec3_t<T> &b, const vec3_t<T> &c) {
            if constexpr (rowMajor) {
                h = {a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z};
            } else {
                h = {a.x, b.x, c.x, a.y, b.y, c.y, a.z, b.z, c.z};
            }
        }
        inline mat3_t(const T d[9]) : data{d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[9]} {}
        inline vec3_t<T> Row1() const { return vec3_t<T>(h.x1, h.y1, h.z1); }
        inline vec3_t<T> Row2() const { return vec3_t<T>(h.x2, h.y2, h.z2); }
        inline vec3_t<T> Row3() const { return vec3_t<T>(h.x3, h.y3, h.z3); }
        inline vec3_t<T> Col1() const { return vec3_t<T>(v.x1, v.y1, v.z1); }
        inline vec3_t<T> Col2() const { return vec3_t<T>(v.x2, v.y2, v.z2); }
        inline vec3_t<T> Col3() const { return vec3_t<T>(v.x3, v.y3, v.z3); }
        inline static mat3_t<T> Identity() {
            return mat3_t(1);
        };
        // Only useful for rotations about aligned axes, such as {1, 0, 0}
        static mat3_t<T> RotationBasic(T angle, Axis axis);
        // Useful for arbitrary axes
        static mat3_t<T> Rotation(T angle, vec3_t<T> axis);
        static mat3_t<T> Scaler(vec3_t<T> scale);
        inline mat3_t<T> Transpose() const {
            return mat3_t<T>(v.x1, v.y1, v.z1, v.x2, v.y2, v.z2, v.x3, v.y3, v.z3);
        }
        inline mat3_t<T> operator+(const mat3_t<T>& a) const {
            return mat3_t<T>(
                h.x1 + a.h.x1, h.y1 + a.h.y1, h.z1 + a.h.z1,
                h.x2 + a.h.x2, h.y2 + a.h.y2, h.z2 + a.h.z2,
                h.x3 + a.h.x3, h.y3 + a.h.y3, h.z3 + a.h.z3
            );
        }
        inline mat3_t<T> operator*(const mat3_t<T>& a) const {
            return mat3_t<T>(
                h.x1 * a.v.x1 + h.y1 * a.v.y1 + h.z1 * a.v.z1,
                h.x1 * a.v.x2 + h.y1 * a.v.y2 + h.z1 * a.v.z2,
                h.x1 * a.v.x3 + h.y1 * a.v.y3 + h.z1 * a.v.z3,
                h.x2 * a.v.x1 + h.y2 * a.v.y1 + h.z2 * a.v.z1,
                h.x2 * a.v.x2 + h.y2 * a.v.y2 + h.z2 * a.v.z2,
                h.x2 * a.v.x3 + h.y2 * a.v.y3 + h.z2 * a.v.z3,
                h.x3 * a.v.x1 + h.y3 * a.v.y1 + h.z3 * a.v.z1,
                h.x3 * a.v.x2 + h.y3 * a.v.y2 + h.z3 * a.v.z2,
                h.x3 * a.v.x3 + h.y3 * a.v.y3 + h.z3 * a.v.z3
            );
        }
        inline vec3_t<T> operator*(const vec3_t<T>& a) const {
            return vec3_t<T>(
                h.x1*a.x + h.y1*a.y + h.z1*a.z,
                h.x2*a.x + h.y2*a.y + h.z2*a.z,
                h.x3*a.x + h.y3*a.y + h.z3*a.z
            );
        }
        inline mat3_t<T> operator*(const T& a) const {
            return mat3_t<T>(
                h.x1*a, h.y1*a, h.z1*a,
                h.x2*a, h.y2*a, h.z2*a,
                h.x3*a, h.y3*a, h.z3*a
            );
        }
        inline mat3_t<T> operator/(const T& a) const {
            return mat3_t<T>(
                h.x1/a, h.y1/a, h.z1/a,
                h.x2/a, h.y2/a, h.z2/a,
                h.x3/a, h.y3/a, h.z3/a
            );
        }
    };

    template<typename T>
    inline vec3_t<T> operator*(const vec3_t<T> &a, const mat3_t<T> &b) {
        return vec3_t<T>(
            a.x*b.v.x1 + a.y*b.v.y1 + a.z*b.v.z1,
            a.x*b.v.x2 + a.y*b.v.y2 + a.z*b.v.z2,
            a.x*b.v.x3 + a.y*b.v.y3 + a.z*b.v.z3
        );
    }
#endif // MATH_MAT3

#ifdef MATH_VEC4
    template<typename T>
    struct vec4_t {
        union {
            struct {
                T x, y, z, w;
            };
            struct {
                T r, g, b, a;
            };
            struct {
                T h, s, v;
            };
    #ifdef MATH_VEC3
            struct {
                vec3_t<T> xyz;
            };
            struct {
                vec3_t<T> rgb;
            };
            struct {
                vec3_t<T> hsv;
            };
    #endif
            struct {
                T data[4];
            };
        };

        vec4_t() = default;
        inline vec4_t(const T &vec) : x(vec) , y (vec) , z(vec) , w(vec) {}
        inline vec4_t(const T &v1, const T &v2, const T &v3, const T &v4) : x(v1) , y(v2) , z(v3) , w(v4) {}
#ifdef MATH_VEC3
        inline vec4_t(const vec3_t<T> &vec, const T &v1) : r(vec.r), g(vec.g), b(vec.b), a(v1) {}
#endif
        template<typename I>
        vec4_t(const vec4_t<I> &a) : x((T)a.x) , y((T)a.y), z((T)a.z), w((T)a.w) {}
        inline vec4_t<T> operator+(const vec4_t<T>& vec) const { return vec4_t<T>(x+vec.x, y+vec.y, z+vec.z, w+vec.w); }
        inline vec4_t<T> operator-(const vec4_t<T>& vec) const { return vec4_t<T>(x-vec.x, y-vec.y, z-vec.z, w-vec.w); }
        inline vec4_t<T> operator-() const { return vec4_t<T>(-x, -y, -z, -w); }
        inline vec4_t<T> operator*(const vec4_t<T>& vec) const { return vec4_t<T>(x*vec.x, y*vec.y, z*vec.z, w*vec.w); }
        inline vec4_t<T> operator/(const vec4_t<T>& vec) const { return vec4_t<T>(x/vec.x, y/vec.y, z/vec.z, w/vec.w); }
        inline vec4_t<T> operator*(const T& vec) const { return vec4_t<T>(x*vec, y*vec, z*vec, w*vec); }
        inline vec4_t<T> operator/(const T& vec) const { return vec4_t<T>(x/vec, y/vec, z/vec, w/vec); }
        inline bool operator==(const vec4_t<T>& a) const { return x == a.x && y == a.y && z == a.z && w == a.w; }
        inline bool operator!=(const vec4_t<T>& a) const { return x != a.x || y != a.y || z != a.z || w != a.w; }
        inline T& operator[](const u32& i) { return data[i]; }
        vec4_t<T> operator+=(const vec4_t<T>& vec);
        vec4_t<T> operator-=(const vec4_t<T>& vec);
        vec4_t<T> operator/=(const vec4_t<T>& vec);
        vec4_t<T> operator/=(const T& vec);
        vec4_t<T> operator*=(const vec4_t<T>& vec);
        vec4_t<T> operator*=(const T& vec);
    };

    template<typename T>
    inline vec4_t<T> operator*(const T& a, const vec4_t<T>& b) {
        return b * a;
    }

    template<typename T>
    inline T dot(const vec4_t<T>& a, const vec4_t<T>& b) {
        return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
    }

    template<typename T>
    inline T absSqr(const vec4_t<T>& a) {
        return a.x*a.x + a.y*a.y + a.z*a.z + a.w*a.w;
    }

    template<typename T>
    inline T abs(const vec4_t<T>& a) {
        return sqrt(a.x*a.x + a.y*a.y + a.z*a.z + a.w*a.w);
    }

    template<bool isSegment, typename T>
    T distSqrToLine(const vec4_t<T> &segA, const vec4_t<T> &segB, const vec4_t<T> &point) {
        const vec4_t<T> diff = segA - segB;
        const T lengthSquared = absSqr(diff);
        const T t = dot(diff, segA - point) / lengthSquared;
        vec4_t<T> projection;
        if constexpr (isSegment) {
            if (t < 0.0) {
                projection = segA;
            } else if (t > 1.0) {
                projection = segB;
            } else {
                projection = segA - diff * t;
            }
        } else {
            projection = segA - diff * t;
        }
        return absSqr(point - projection);
    }
#endif // MATH_VEC4

#ifdef MATH_MAT4
    template<typename T>
    struct mat4_t {
        union {
            struct {
                T x1, y1, z1, w1,
                  x2, y2, z2, w2,
                  x3, y3, z3, w3,
                  x4, y4, z4, w4;
            } h;
            struct {
                T x1, x2, x3, x4,
                  y1, y2, y3, y4,
                  z1, z2, z3, z4,
                  w1, w2, w3, w4;
            } v;
            struct {
                T data[16];
            };
        };
        mat4_t() = default;
        inline mat4_t(const T &a) : h{a, 0, 0, 0, 0, a, 0, 0, 0, 0, a, 0, 0, 0, 0, a} {}
        inline mat4_t(const T &x1, const T &y1, const T &z1, const T &w1,
                      const T &x2, const T &y2, const T &z2, const T &w2,
                      const T &x3, const T &y3, const T &z3, const T &w3,
                      const T &x4, const T &y4, const T &z4, const T &w4) :
                data{x1, y1, z1, w1, x2, y2, z2, w2, x3, y3, z3, w3, x4, y4, z4, w4} {}
        template<bool rowMajor = true>
        inline mat4_t(const vec4_t<T> &a, const vec4_t<T> &b, const vec4_t<T> &c, const vec4_t<T> &d) {
            if constexpr (rowMajor) {
                h = {a.x, a.y, a.z, a.w, b.x, b.y, b.z, b.w, c.x, c.y, c.z, c.w, d.x, d.y, d.z, d.w};
            } else {
                h = {a.x, b.x, c.x, d.x, a.y, b.y, c.y, d.y, a.z, b.z, c.z, d.z, a.w, b.w, c.w, d.w};
            }
        }
        inline mat4_t(const T d[16]) : data{d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[9], d[10], d[11], d[12], d[13], d[14], d[15]} {}
        inline vec4_t<T> Row1() const { return vec4_t<T>(h.x1, h.y1, h.z1, h.w1); }
        inline vec4_t<T> Row2() const { return vec4_t<T>(h.x2, h.y2, h.z2, h.w2); }
        inline vec4_t<T> Row3() const { return vec4_t<T>(h.x3, h.y3, h.z3, h.w3); }
        inline vec4_t<T> Row4() const { return vec4_t<T>(h.x4, h.y4, h.z4, h.w4); }
        inline vec4_t<T> Col1() const { return vec4_t<T>(v.x1, v.y1, v.z1, v.w1); }
        inline vec4_t<T> Col2() const { return vec4_t<T>(v.x2, v.y2, v.z2, v.w2); }
        inline vec4_t<T> Col3() const { return vec4_t<T>(v.x3, v.y3, v.z3, v.w3); }
        inline vec4_t<T> Col4() const { return vec4_t<T>(v.x4, v.y4, v.z4, v.w4); }
        inline static mat4_t<T> Identity() {
            return mat4_t(1);
        };
        // Only useful for rotations about aligned planes, such as {{1, 0, 0, 0}, {0, 0, 0, 1}}
        // Note: The planes stay fixed in place and everything else rotates around them.
        static mat4_t<T> RotationBasic(T angle, Plane plane);
        // For using 3D-axis rotations
        static mat4_t<T> RotationBasic(T angle, Axis axis);
        // Useful for arbitrary 3D-axes
        static mat4_t<T> Rotation(T angle, vec3_t<T> axis);
        static mat4_t<T> Scaler(vec4_t<T> scale);
        inline mat4_t<T> Transpose() const {
            return mat4_t<T>(v.x1, v.y1, v.z1, v.w1,
                             v.x2, v.y2, v.z2, v.w2,
                             v.x3, v.y3, v.z3, v.w3,
                             v.x4, v.y4, v.z4, v.w4);
        }
        inline mat4_t<T> operator+(const mat4_t<T>& a) const {
            return mat4_t<T>(
                h.x1 + a.h.x1, h.y1 + a.h.y1, h.z1 + a.h.z1, h.w1 + a.h.w1,
                h.x2 + a.h.x2, h.y2 + a.h.y2, h.z2 + a.h.z2, h.w2 + a.h.w2,
                h.x3 + a.h.x3, h.y3 + a.h.y3, h.z3 + a.h.z3, h.w3 + a.h.w3,
                h.x4 + a.h.x4, h.y4 + a.h.y4, h.z4 + a.h.z4, h.w4 + a.h.w4
            );
        }
        inline mat4_t<T> operator*(const mat4_t<T>& a) const {
            return mat4_t<T>(
                h.x1 * a.v.x1 + h.y1 * a.v.y1 + h.z1 * a.v.z1 + h.w1 * a.v.w1,
                h.x1 * a.v.x2 + h.y1 * a.v.y2 + h.z1 * a.v.z2 + h.w1 * a.v.w2,
                h.x1 * a.v.x3 + h.y1 * a.v.y3 + h.z1 * a.v.z3 + h.w1 * a.v.w3,
                h.x1 * a.v.x4 + h.y1 * a.v.y4 + h.z1 * a.v.z4 + h.w1 * a.v.w4,
                h.x2 * a.v.x1 + h.y2 * a.v.y1 + h.z2 * a.v.z1 + h.w2 * a.v.w1,
                h.x2 * a.v.x2 + h.y2 * a.v.y2 + h.z2 * a.v.z2 + h.w2 * a.v.w2,
                h.x2 * a.v.x3 + h.y2 * a.v.y3 + h.z2 * a.v.z3 + h.w2 * a.v.w3,
                h.x2 * a.v.x4 + h.y2 * a.v.y4 + h.z2 * a.v.z4 + h.w2 * a.v.w4,
                h.x3 * a.v.x1 + h.y3 * a.v.y1 + h.z3 * a.v.z1 + h.w3 * a.v.w1,
                h.x3 * a.v.x2 + h.y3 * a.v.y2 + h.z3 * a.v.z2 + h.w3 * a.v.w2,
                h.x3 * a.v.x3 + h.y3 * a.v.y3 + h.z3 * a.v.z3 + h.w3 * a.v.w3,
                h.x3 * a.v.x4 + h.y3 * a.v.y4 + h.z3 * a.v.z4 + h.w3 * a.v.w4,
                h.x4 * a.v.x1 + h.y4 * a.v.y1 + h.z4 * a.v.z1 + h.w4 * a.v.w1,
                h.x4 * a.v.x2 + h.y4 * a.v.y2 + h.z4 * a.v.z2 + h.w4 * a.v.w2,
                h.x4 * a.v.x3 + h.y4 * a.v.y3 + h.z4 * a.v.z3 + h.w4 * a.v.w3,
                h.x4 * a.v.x4 + h.y4 * a.v.y4 + h.z4 * a.v.z4 + h.w4 * a.v.w4
            );
        }
        inline vec4_t<T> operator*(const vec4_t<T>& a) const {
            return vec4_t<T>(
                h.x1*a.x + h.y1*a.y + h.z1*a.z + h.w1*a.w,
                h.x2*a.x + h.y2*a.y + h.z2*a.z + h.w2*a.w,
                h.x3*a.x + h.y3*a.y + h.z3*a.z + h.w3*a.w,
                h.x4*a.x + h.y4*a.y + h.z4*a.z + h.w4*a.w
            );
        }
        inline mat4_t<T> operator*(const T& a) const {
            return mat4_t<T>(
                h.x1*a, h.y1*a, h.z1*a, h.w1*a,
                h.x2*a, h.y2*a, h.z2*a, h.w2*a,
                h.x3*a, h.y3*a, h.z3*a, h.w3*a,
                h.x4*a, h.y4*a, h.z4*a, h.w4*a
            );
        }
        inline mat4_t<T> operator/(const T& a) const {
            return mat4_t<T>(
                h.x1/a, h.y1/a, h.z1/a, h.w1/a,
                h.x2/a, h.y2/a, h.z2/a, h.w2/a,
                h.x3/a, h.y3/a, h.z3/a, h.w3/a,
                h.x4/a, h.y4/a, h.z4/a, h.w4/a
            );
        }
    };

    template<typename T>
    inline vec4_t<T> operator*(const vec4_t<T> &a, const mat4_t<T> &b) {
        return vec4_t<T>(
            a.x*b.v.x1 + a.y*b.v.y1 + a.z*b.v.z1 + a.w*b.v.w1,
            a.x*b.v.x2 + a.y*b.v.y2 + a.z*b.v.z2 + a.w*b.v.w2,
            a.x*b.v.x3 + a.y*b.v.y3 + a.z*b.v.z3 + a.w*b.v.w3,
            a.x*b.v.x4 + a.y*b.v.y4 + a.z*b.v.z4 + a.w*b.v.w4
        );
    }
#endif // MATH_MAT4

#ifdef MATH_VEC5
    template<typename T>
    struct vec5_t {
        union {
            struct {
                T x, y, z, w, v;
            };
            struct {
                T data[5];
            };
        };

        vec5_t() = default;
        inline vec5_t(const T &vec) : x(vec) , y (vec) , z(vec) , w(vec) , v(vec) {}
        inline vec5_t(const T &v1, const T &v2, const T &v3, const T &v4, const T &v5) :
                x(v1) , y(v2) , z(v3) , w(v4) , v(v5) {}
        template<typename I>
        vec5_t(const vec5_t<I> &a) : x((T)a.x) , y((T)a.y), z((T)a.z), w((T)a.w), v((T)a.v) {}
        inline vec5_t<T> operator+(const vec5_t<T>& vec) const { return vec5_t<T>(x+vec.x, y+vec.y, z+vec.z, w+vec.w, v+vec.v); }
        inline vec5_t<T> operator-(const vec5_t<T>& vec) const { return vec5_t<T>(x-vec.x, y-vec.y, z-vec.z, w-vec.w, v-vec.v); }
        inline vec5_t<T> operator-() const { return vec5_t<T>(-x, -y, -z, -w, -v); }
        inline vec5_t<T> operator*(const vec5_t<T>& vec) const { return vec5_t<T>(x*vec.x, y*vec.y, z*vec.z, w*vec.w, v*vec.v); }
        inline vec5_t<T> operator/(const vec5_t<T>& vec) const { return vec5_t<T>(x/vec.x, y/vec.y, z/vec.z, w/vec.w, v/vec.v); }
        inline vec5_t<T> operator*(const T& vec) const { return vec5_t<T>(x*vec, y*vec, z*vec, w*vec, v*vec); }
        inline vec5_t<T> operator/(const T& vec) const { return vec5_t<T>(x/vec, y/vec, z/vec, w/vec, v/vec); }
        inline bool operator==(const vec4_t<T>& a) const { return x == a.x && y == a.y && z == a.z && w == a.w && v == a.v; }
        inline bool operator!=(const vec4_t<T>& a) const { return x != a.x || y != a.y || z != a.z || w != a.w || v != a.v; }
        inline T& operator[](const u32& i) { return data[i]; }
        vec5_t<T> operator+=(const vec5_t<T>& vec);
        vec5_t<T> operator-=(const vec5_t<T>& vec);
        vec5_t<T> operator/=(const vec5_t<T>& vec);
        vec5_t<T> operator/=(const T& vec);
        vec5_t<T> operator*=(const vec5_t<T>& vec);
        vec5_t<T> operator*=(const T& vec);
    };
    template<typename T>
    inline vec5_t<T> operator*(const T& a, const vec5_t<T>& b) {
        return b * a;
    }

    template<typename T>
    inline T dot(const vec5_t<T>& a, const vec5_t<T>& b) {
        return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w + a.v*b.v;
    }

    template<typename T>
    inline T absSqr(const vec5_t<T>& a) {
        return a.x*a.x + a.y*a.y + a.z*a.z + a.w*a.w + a.v*a.v;
    }

    template<typename T>
    inline T abs(const vec5_t<T>& a) {
        return sqrt(a.x*a.x + a.y*a.y + a.z*a.z + a.w*a.w + a.v*a.v);
    }

    template<bool isSegment, typename T>
    T distSqrToLine(const vec5_t<T> &segA, const vec5_t<T> &segB, const vec5_t<T> &point) {
        const vec5_t<T> diff = segA - segB;
        const T lengthSquared = absSqr(diff);
        const T t = dot(diff, segA - point) / lengthSquared;
        vec5_t<T> projection;
        if constexpr (isSegment) {
            if (t < 0.0) {
                projection = segA;
            } else if (t > 1.0) {
                projection = segB;
            } else {
                projection = segA - diff * t;
            }
        } else {
            projection = segA - diff * t;
        }
        return absSqr(point - projection);
    }
#endif // MATH_VEC5

#ifdef MATH_MAT5
    template<typename T>
    struct mat5_t {
        union {
            struct {
                T x1, y1, z1, w1, v1,
                  x2, y2, z2, w2, v2,
                  x3, y3, z3, w3, v3,
                  x4, y4, z4, w4, v4,
                  x5, y5, z5, w5, v5;
            } h;
            struct {
                T x1, x2, x3, x4, x5,
                  y1, y2, y3, y4, y5,
                  z1, z2, z3, z4, z5,
                  w1, w2, w3, w4, w5,
                  v1, v2, v3, v4, v5;
            } v;
            struct {
                T data[25];
            };
        };
        mat5_t() = default;
        inline mat5_t(const T &a) : h{a, 0, 0, 0, 0, 0, a, 0, 0, 0, 0, 0, a, 0, 0, 0, 0, 0, a, 0, 0, 0, 0, 0, a} {}
        inline mat5_t(const T &x1, const T &y1, const T &z1, const T &w1, const T &v1,
                      const T &x2, const T &y2, const T &z2, const T &w2, const T &v2,
                      const T &x3, const T &y3, const T &z3, const T &w3, const T &v3,
                      const T &x4, const T &y4, const T &z4, const T &w4, const T &v4,
                      const T &x5, const T &y5, const T &z5, const T &w5, const T &v5) :
                data{x1, y1, z1, w1, v1, x2, y2, z2, w2, v2, x3, y3, z3, w3, v3,
                     x4, y4, z4, w4, v4, x5, y5, z5, w5, v5} {}
        template<bool rowMajor = true>
        inline mat5_t(const vec5_t<T> &a, const vec5_t<T> &b, const vec5_t<T> &c, const vec5_t<T> &d, const vec5_t<T> &e) {
            if constexpr (rowMajor) {
                h = {a.x, a.y, a.z, a.w, a.v,
                     b.x, b.y, b.z, b.w, b.v,
                     c.x, c.y, c.z, c.w, c.v,
                     d.x, d.y, d.z, d.w, d.v,
                     e.x, e.y, e.z, e.w, e.v};
            } else {
                h = {a.x, b.x, c.x, d.x, e.x,
                     a.y, b.y, c.y, d.y, e.y,
                     a.z, b.z, c.z, d.z, e.z,
                     a.w, b.w, c.w, d.w, e.w,
                     a.v, b.v, c.v, d.v, e.v};
            }
        }
        inline mat5_t(const T d[25]) :
                data{d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[9], d[10], d[11], d[12], d[13],
                     d[14], d[15], d[16], d[17], d[18], d[19], d[20], d[21], d[22], d[23], d[24]} {}
        inline vec5_t<T> Row1() const { return vec5_t<T>(h.x1, h.y1, h.z1, h.w1, h.v1); }
        inline vec5_t<T> Row2() const { return vec5_t<T>(h.x2, h.y2, h.z2, h.w2, h.v2); }
        inline vec5_t<T> Row3() const { return vec5_t<T>(h.x3, h.y3, h.z3, h.w3, h.v3); }
        inline vec5_t<T> Row4() const { return vec5_t<T>(h.x4, h.y4, h.z4, h.w4, h.v4); }
        inline vec5_t<T> Row5() const { return vec5_t<T>(h.x5, h.y5, h.z5, h.w5, h.v5); }
        inline vec5_t<T> Col1() const { return vec5_t<T>(v.x1, v.y1, v.z1, v.w1, v.v1); }
        inline vec5_t<T> Col2() const { return vec5_t<T>(v.x2, v.y2, v.z2, v.w2, v.v2); }
        inline vec5_t<T> Col3() const { return vec5_t<T>(v.x3, v.y3, v.z3, v.w3, v.v3); }
        inline vec5_t<T> Col4() const { return vec5_t<T>(v.x4, v.y4, v.z4, v.w4, v.v4); }
        inline vec5_t<T> Col5() const { return vec5_t<T>(v.x5, v.y5, v.z5, v.w5, v.v5); }
        inline static mat5_t<T> Identity() {
            return mat5_t(1);
        };
        // Only useful for rotations about aligned planes, such as {{1, 0, 0, 0}, {0, 0, 0, 1}}
        // Note: The planes stay fixed in place and everything else rotates around them.
        // Note: The V-Dimension is always fixed in place
        static mat5_t<T> RotationBasic(T angle, Plane plane);
        // For using 3D-axis rotations
        static mat5_t<T> RotationBasic(T angle, Axis axis);
        // Useful for arbitrary 3D-axes
        static mat5_t<T> Rotation(T angle, vec3_t<T> axis);
        static mat5_t<T> Scaler(vec5_t<T> scale);
        inline mat5_t<T> Transpose() const {
            return mat5_t<T>(v.x1, v.y1, v.z1, v.w1, v.v1,
                             v.x2, v.y2, v.z2, v.w2, v.v2,
                             v.x3, v.y3, v.z3, v.w3, v.v3,
                             v.x4, v.y4, v.z4, v.w4, v.v4,
                             v.x5, v.y5, v.z5, v.w5, v.v5);
        }
        inline mat5_t<T> operator+(const mat5_t<T>& a) const {
            return mat5_t<T>(
                h.x1 + a.h.x1, h.y1 + a.h.y1, h.z1 + a.h.z1, h.w1 + a.h.w1, h.v1 + a.h.v1,
                h.x2 + a.h.x2, h.y2 + a.h.y2, h.z2 + a.h.z2, h.w2 + a.h.w2, h.v2 + a.h.v2,
                h.x3 + a.h.x3, h.y3 + a.h.y3, h.z3 + a.h.z3, h.w3 + a.h.w3, h.v3 + a.h.v3,
                h.x4 + a.h.x4, h.y4 + a.h.y4, h.z4 + a.h.z4, h.w4 + a.h.w4, h.v4 + a.h.v4,
                h.x5 + a.h.x5, h.y5 + a.h.y5, h.z5 + a.h.z5, h.w5 + a.h.w5, h.v5 + a.h.v5
            );
        }
        inline mat5_t<T> operator*(const mat5_t<T>& a) const {
            return mat5_t<T>(
                h.x1 * a.v.x1 + h.y1 * a.v.y1 + h.z1 * a.v.z1 + h.w1 * a.v.w1 + h.v1 * a.v.v1,
                h.x1 * a.v.x2 + h.y1 * a.v.y2 + h.z1 * a.v.z2 + h.w1 * a.v.w2 + h.v1 * a.v.v2,
                h.x1 * a.v.x3 + h.y1 * a.v.y3 + h.z1 * a.v.z3 + h.w1 * a.v.w3 + h.v1 * a.v.v3,
                h.x1 * a.v.x4 + h.y1 * a.v.y4 + h.z1 * a.v.z4 + h.w1 * a.v.w4 + h.v1 * a.v.v4,
                h.x1 * a.v.x5 + h.y1 * a.v.y5 + h.z1 * a.v.z5 + h.w1 * a.v.w5 + h.v1 * a.v.v5,
                h.x2 * a.v.x1 + h.y2 * a.v.y1 + h.z2 * a.v.z1 + h.w2 * a.v.w1 + h.v2 * a.v.v1,
                h.x2 * a.v.x2 + h.y2 * a.v.y2 + h.z2 * a.v.z2 + h.w2 * a.v.w2 + h.v2 * a.v.v2,
                h.x2 * a.v.x3 + h.y2 * a.v.y3 + h.z2 * a.v.z3 + h.w2 * a.v.w3 + h.v2 * a.v.v3,
                h.x2 * a.v.x4 + h.y2 * a.v.y4 + h.z2 * a.v.z4 + h.w2 * a.v.w4 + h.v2 * a.v.v4,
                h.x2 * a.v.x5 + h.y2 * a.v.y5 + h.z2 * a.v.z5 + h.w2 * a.v.w5 + h.v2 * a.v.v5,
                h.x3 * a.v.x1 + h.y3 * a.v.y1 + h.z3 * a.v.z1 + h.w3 * a.v.w1 + h.v3 * a.v.v1,
                h.x3 * a.v.x2 + h.y3 * a.v.y2 + h.z3 * a.v.z2 + h.w3 * a.v.w2 + h.v3 * a.v.v2,
                h.x3 * a.v.x3 + h.y3 * a.v.y3 + h.z3 * a.v.z3 + h.w3 * a.v.w3 + h.v3 * a.v.v3,
                h.x3 * a.v.x4 + h.y3 * a.v.y4 + h.z3 * a.v.z4 + h.w3 * a.v.w4 + h.v3 * a.v.v4,
                h.x3 * a.v.x5 + h.y3 * a.v.y5 + h.z3 * a.v.z5 + h.w3 * a.v.w5 + h.v3 * a.v.v5,
                h.x4 * a.v.x1 + h.y4 * a.v.y1 + h.z4 * a.v.z1 + h.w4 * a.v.w1 + h.v4 * a.v.v1,
                h.x4 * a.v.x2 + h.y4 * a.v.y2 + h.z4 * a.v.z2 + h.w4 * a.v.w2 + h.v4 * a.v.v2,
                h.x4 * a.v.x3 + h.y4 * a.v.y3 + h.z4 * a.v.z3 + h.w4 * a.v.w3 + h.v4 * a.v.v3,
                h.x4 * a.v.x4 + h.y4 * a.v.y4 + h.z4 * a.v.z4 + h.w4 * a.v.w4 + h.v4 * a.v.v4,
                h.x4 * a.v.x5 + h.y4 * a.v.y5 + h.z4 * a.v.z5 + h.w4 * a.v.w5 + h.v4 * a.v.v5,
                h.x5 * a.v.x1 + h.y5 * a.v.y1 + h.z5 * a.v.z1 + h.w5 * a.v.w1 + h.v5 * a.v.v1,
                h.x5 * a.v.x2 + h.y5 * a.v.y2 + h.z5 * a.v.z2 + h.w5 * a.v.w2 + h.v5 * a.v.v2,
                h.x5 * a.v.x3 + h.y5 * a.v.y3 + h.z5 * a.v.z3 + h.w5 * a.v.w3 + h.v5 * a.v.v3,
                h.x5 * a.v.x4 + h.y5 * a.v.y4 + h.z5 * a.v.z4 + h.w5 * a.v.w4 + h.v5 * a.v.v4,
                h.x5 * a.v.x5 + h.y5 * a.v.y5 + h.z5 * a.v.z5 + h.w5 * a.v.w5 + h.v5 * a.v.v5
            );
        }
        inline vec5_t<T> operator*(const vec5_t<T>& a) const {
            return vec5_t<T>(
                h.x1*a.x + h.y1*a.y + h.z1*a.z + h.w1*a.w + h.v1*a.v,
                h.x2*a.x + h.y2*a.y + h.z2*a.z + h.w2*a.w + h.v2*a.v,
                h.x3*a.x + h.y3*a.y + h.z3*a.z + h.w3*a.w + h.v3*a.v,
                h.x4*a.x + h.y4*a.y + h.z4*a.z + h.w4*a.w + h.v4*a.v,
                h.x5*a.x + h.y5*a.y + h.z5*a.z + h.w5*a.w + h.v5*a.v
            );
        }
        inline mat5_t<T> operator*(const T& a) const {
            return mat5_t<T>(
                h.x1*a, h.y1*a, h.z1*a, h.w1*a, h.v1*a,
                h.x2*a, h.y2*a, h.z2*a, h.w2*a, h.v2*a,
                h.x3*a, h.y3*a, h.z3*a, h.w3*a, h.v3*a,
                h.x4*a, h.y4*a, h.z4*a, h.w4*a, h.v4*a,
                h.x5*a, h.y5*a, h.z5*a, h.w5*a, h.v5*a
            );
        }
        inline mat5_t<T> operator/(const T& a) const {
            return mat5_t<T>(
                h.x1/a, h.y1/a, h.z1/a, h.w1/a, h.v1/a,
                h.x2/a, h.y2/a, h.z2/a, h.w2/a, h.v2/a,
                h.x3/a, h.y3/a, h.z3/a, h.w3/a, h.v3/a,
                h.x4/a, h.y4/a, h.z4/a, h.w4/a, h.v4/a,
                h.x5/a, h.y5/a, h.z5/a, h.w5/a, h.v5/a
            );
        }
    };

    template<typename T>
    inline vec5_t<T> operator*(const vec5_t<T> &a, const mat5_t<T> &b) {
        return vec5_t<T>(
            a.x*b.v.x1 + a.y*b.v.y1 + a.z*b.v.z1 + a.w*b.v.w1 + a.v*b.v.v1,
            a.x*b.v.x2 + a.y*b.v.y2 + a.z*b.v.z2 + a.w*b.v.w2 + a.v*b.v.v2,
            a.x*b.v.x3 + a.y*b.v.y3 + a.z*b.v.z3 + a.w*b.v.w3 + a.v*b.v.v3,
            a.x*b.v.x4 + a.y*b.v.y4 + a.z*b.v.z4 + a.w*b.v.w4 + a.v*b.v.v4,
            a.x*b.v.x5 + a.y*b.v.y5 + a.z*b.v.z5 + a.w*b.v.w5 + a.v*b.v.v5
        );
    }
#endif // MATH_MAT5

#ifdef MATH_COMPLEX
    template<typename T>
    struct complex_t {
        union {
            struct {
                T real, imag;
            };
#ifdef MATH_VEC2
            struct {
                vec2_t<T> vector;
            };
#endif
            struct {
                T x, y;
            };
            struct {
                T data[2];
            };
        };

        complex_t() = default;
        inline complex_t(const T &a) : x(a) , y(0) {}
        inline complex_t(const T &a, const T &b) : x(a) , y(b) {}
#ifdef MATH_VEC2
        inline complex_t(const vec2_t<T> &vec) : vector(vec) {}
#endif
        inline complex_t(const T d[2]) : x(d[0]) , y(d[1]) {}

        inline complex_t<T> operator*(const complex_t<T>& a) const {
            return complex_t<T>(real*a.real - imag*a.imag, real*a.imag + imag*a.real);
        }
        inline complex_t<T> operator*(const T& a) const {
            return complex_t<T>(real*a, imag*a);
        }
        inline complex_t<T> operator/(const complex_t<T>& a) const {
            return (*this) * a.Reciprocal();
        }
        inline complex_t<T> operator/(const T& a) const {
            return complex_t<T>(real/a, imag/a);
        }
        inline complex_t<T> operator+(const complex_t<T>& a) const {
            return complex_t<T>(real + a.real, imag + a.imag);
        }
        inline complex_t<T> operator+(const T& a) const {
            return complex_t<T>(real + a, imag);
        }
        inline complex_t<T> operator-(const complex_t<T>& a) const {
            return complex_t<T>(real - a.real, imag - a.imag);
        }
        inline complex_t<T> operator-(const T& a) const {
            return complex_t<T>(real - a, imag);
        }
        inline complex_t<T> operator-() const {
            return complex_t<T>(-real, -imag);
        }
        complex_t<T>& operator+=(const complex_t<T>& a);
        complex_t<T>& operator-=(const complex_t<T>& a);
        complex_t<T>& operator*=(const complex_t<T>& a);
        complex_t<T>& operator/=(const complex_t<T>& a);
        complex_t<T>& operator+=(const T& a);
        complex_t<T>& operator-=(const T& a);
        complex_t<T>& operator*=(const T& a);
        complex_t<T>& operator/=(const T& a);
        inline complex_t<T> Conjugate() const {
            return complex_t<T>(real, -imag);
        }
        inline complex_t<T> Reciprocal() const {
            return Conjugate() / (x*x + y*y);
        }
    };

    template<typename T>
    inline complex_t<T> operator*(const T& a, const complex_t<T>& b) {
        return b*a;
    }

    template<typename T>
    inline complex_t<T> operator/(const T& a, const complex_t<T>& b) {
        return complex_t<T>(a)/b;
    }

    template<typename T>
    inline complex_t<T> operator+(const T& a, const complex_t<T>& b) {
        return b+a;
    }

    template<typename T>
    inline complex_t<T> operator-(const T& a, const complex_t<T>& b) {
        return -b+a;
    }

    template<typename T>
    inline T abs(const complex_t<T>& a) {
        return sqrt(a.x*a.x + a.y*a.y);
    }

    template<typename T>
    complex_t<T> exp(const complex_t<T>& a);

    template<typename T>
    complex_t<T> log(const complex_t<T>& a);

    template<typename T>
    complex_t<T> pow(const complex_t<T>& a, const complex_t<T>& e);

    template<typename T>
    complex_t<T> pow(const complex_t<T>& a, const T& e);

#endif // MATH_COMPLEX

#ifdef MATH_QUATERNION
    template<typename T>
    struct quat_t {
        union {
            struct {
                T w, x, y, z;
            };
            struct {
                T scalar;
                vec3_t<T> vector;
            };
#ifdef MATH_VEC4
            struct {
                vec4_t<T> wxyz;
            };
#endif
            struct {
                T data[4];
            };
        };

        quat_t() = default;
        inline quat_t(const T &a) : data{a, 0, 0, 0} {}
        inline quat_t(const T &a, const vec3_t<T> &v) : scalar(a), vector(v) {}
#ifdef MATH_VEC4
        inline quat_t(const vec4_t<T> &v) : wxyz(v) {}
#endif
        inline quat_t(const T &a, const T &b, const T &c, const T &d) : w(a), x(b), y(c), z(d) {}
        inline quat_t(const T d[4]) : data{d[0], d[1], d[2], d[3]} {}

        inline quat_t<T> operator*(const quat_t<T>& a) const {
            return quat_t<T>(
                w*a.w - x*a.x - y*a.y - z*a.z,
                w*a.x + x*a.w + y*a.z - z*a.y,
                w*a.y - x*a.z + y*a.w + z*a.x,
                w*a.z + x*a.y - y*a.x + z*a.w
            );
        }
        inline quat_t<T> operator*(const T& a) const {
            return quat_t<T>(w*a, x*a, y*a, z*a);
        }
        inline quat_t<T> operator/(const quat_t<T>& a) const {
            return (*this) * a.Reciprocal();
        }
        inline quat_t<T> operator/(const T& a) const {
            return quat_t<T>(w/a, x/a, y/a, z/a);
        }
        inline quat_t<T> operator-(const quat_t<T>& a) const {
            return quat_t<T>(w-a.w, x-a.x, y-a.y, z-a.z);
        }
        inline quat_t<T> operator+(const quat_t<T>& a) const {
            return quat_t<T>(w+a.w, x+a.x, y+a.y, z+a.z);
        }
        quat_t<T>& operator+=(const quat_t<T>& a);
        quat_t<T>& operator-=(const quat_t<T>& a);
        quat_t<T>& operator*=(const quat_t<T>& a);
        quat_t<T>& operator/=(const quat_t<T>& a);
        quat_t<T>& operator+=(const T& a);
        quat_t<T>& operator-=(const T& a);
        quat_t<T>& operator*=(const T& a);
        quat_t<T>& operator/=(const T& a);
        inline quat_t<T> Conjugate() const {
            return quat_t<T>(scalar, -vector);
        }
        inline T Norm() const {
            return sqrt(w*w + x*x + y*y + z*z);
        }
        inline quat_t<T> Reciprocal() const {
            return Conjugate() / (w*w + x*x + y*y + z*z); // For unit quaternions just use Conjugate()
        }
        // Make a rotation quaternion
        static inline quat_t<T> Rotation(const T& angle, const vec3_t<T>& axis) {
            return quat_t<T>(cos(angle/2.0), normalize(axis) * sin(angle/2.0));
        }
        // A one-off rotation of a point
        static vec3_t<T> RotatePoint(vec3_t<T> point, T angle, vec3_t<T> axis);
        // Using this quaternion for a one-off rotation of a point
        vec3_t<T> RotatePoint(vec3_t<T> point) const;
        // Rotating this quaternion about an axis
        quat_t<T> Rotate(T angle, vec3_t<T> axis) const;
        // Rotate this quaternion by using a specified rotation quaternion
        quat_t<T> Rotate(quat_t<T> rotation) const;
#ifdef MATH_MAT3
        // Convert this rotation quaternion into a matrix
        mat3_t<T> ToMat3() const;
#endif
    };

    template<typename T>
    inline quat_t<T> normalize(const quat_t<T>& a) {
        return a / a.Norm();
    }

    template<typename T>
    quat_t<T> slerp(quat_t<T> a, quat_t<T> b, T factor);

    template<typename T>
    quat_t<T> exp(quat_t<T> a);

    template<typename T>
    quat_t<T> log(quat_t<T> a);

    template<typename T>
    quat_t<T> pow(const quat_t<T>& a, const quat_t<T>& e);

    template<typename T>
    quat_t<T> pow(const quat_t<T>& a, const T& e);

#endif // MATH_QUATERNION

#ifdef MATH_EQUATIONS

template<typename T>
struct SolutionLinear {
    T x;
    bool real=false;
};

template<typename T>
SolutionLinear<T> SolveLinear(T a, T b) {
    SolutionLinear<T> solution;
    if (a != 0.0) {
        solution.x = -b / a;
        solution.real = true;
    }
    return solution;
}

template<typename T>
struct SolutionQuadratic {
    T x1, x2;
    bool x1Real=false, x2Real=false;
    SolutionQuadratic() = default;
    SolutionQuadratic(const SolutionLinear<T>& solution) : x1(solution.x) , x1Real(solution.real) {}
};

template<typename T>
SolutionQuadratic<T> SolveQuadratic(T a, T b, T c) {
    SolutionQuadratic<T> solution;
    if (a == 0.0) {
        return SolveLinear<T>(b, c);
    }
    const T bb = square(b);
    const T ac4 = 4.0*a*c;
    if (bb < ac4) {
        // We don't care about complex answers
        return solution;
    }
    if (bb == ac4) {
        solution.x1 = -b / a;
        solution.x1Real = true;
        return solution;
    }
    const T squareRoot = sqrt(bb - ac4);
    a *= 2.0;
    solution.x1 = (-b + squareRoot) / a;
    solution.x1Real = true;
    solution.x2 = (-b - squareRoot) / a;
    solution.x2Real = true;
    return solution;
}

template<typename T>
struct SolutionCubic {
    T x1, x2, x3;
    bool x1Real, x2Real, x3Real;
    SolutionCubic() = default;
    SolutionCubic(const SolutionQuadratic<T>& solution) : x1(solution.x1) , x2(solution.x2) , x1Real(solution.x1Real) , x2Real(solution.x2Real) , x3Real(false) {}
};

template<typename T>
SolutionCubic<T> SolveCubic(T a, T b, T c, T d) {
    SolutionCubic<T> solution;
    if (a == 0.0) {
        return SolveQuadratic<T>(b, c, d);
    }
    // We'll use Cardano's formula
    // First we need to be in terms of the depressed cubic.
    // So we take our current form:
    // ax^3 + bx^2 + cx + d = 0
    // divide by a to get:
    // x^3 + ix^2 + jx + k = 0
    // where i = b/a, j = c/a, and k = d/a
    const T i = b/a;
    const T j = c/a;
    const T k = d/a;
    // substitute t for x to get
    // t^3 + pt + q
    // where t = (x + i/3), p = (j - i^2/3), and q = (k + 2i^3/27 - ij/3)
    const T p = j - i*i / 3.0;
    const T q = -i * (2.0 * i*i - 9.0 * j) / 27.0 - k;
    const T p3 = p*p*p;
    const T sqrD = q*q + p3 * 4.0 / 27.0;
    const T offset = -i / 3.0; // Since t = (x + i/3), x = t - i/3
    if (sqrD > 0.0) {
        // We have a single real solution
        const T rootD = sqrt(sqrD);
        const T u = cubert((q + rootD) * 0.5);
        const T v = cubert((q - rootD) * 0.5);
        solution.x1 = u + v + offset;
        solution.x1Real = true;
        solution.x2Real = false;
        solution.x3Real = false;
    } else if (sqrD < 0.0) {
        // We have 3 real solutions
        const T u = 2.0 * sqrt(-p / 3.0);
        const T v = acos( sqrt(-27.0/p3) * q * 0.5) / 3.0;
        solution.x1 = u * cos(v) + offset;
        solution.x2 = u * cos(v + tau / 3.0) + offset;
        solution.x3 = u * cos(v + tau * 2.0/3.0) + offset;
        solution.x1Real = true;
        solution.x2Real = true;
        solution.x3Real = true;
    } else {
        // We have 2 real solutions
        const T u = cubert(q * 0.5);
        solution.x1 = u * 2.0 + offset;
        solution.x2 = -u + offset;
        solution.x1Real = true;
        solution.x2Real = true;
        solution.x3Real = false;
    }
    return solution;
}

#endif // MATH_EQUATIONS

// Typedefs for nice naming conventions

#ifdef MATH_VEC2
    #ifdef MATH_F32
        typedef vec2_t<f32> vec2;
    #endif
    #ifdef MATH_F64
        typedef vec2_t<f64> vec2d;
    #endif
    typedef vec2_t<i32> vec2i;
#endif
#ifdef MATH_VEC3
    #ifdef MATH_F32
        typedef vec3_t<f32> vec3;
    #endif
    #ifdef MATH_F64
        typedef vec3_t<f64> vec3d;
    #endif
    typedef vec3_t<i32> vec3i;
#endif
#ifdef MATH_VEC4
    #ifdef MATH_F32
        typedef vec4_t<f32> vec4;
    #endif
    #ifdef MATH_F64
        typedef vec4_t<f64> vec4d;
    #endif
    typedef vec4_t<i32> vec4i;
#endif
#ifdef MATH_VEC5
    #ifdef MATH_F32
        typedef vec5_t<f32> vec5;
    #endif
    #ifdef MATH_F64
        typedef vec5_t<f64> vec5d;
    #endif
    typedef vec5_t<i32> vec5i;
#endif
#ifdef MATH_MAT2
    #ifdef MATH_F32
        typedef mat2_t<f32> mat2;
    #endif
    #ifdef MATH_F64
        typedef mat2_t<f64> mat2d;
    #endif
#endif
#ifdef MATH_MAT3
    #ifdef MATH_F32
        typedef mat3_t<f32> mat3;
    #endif
    #ifdef MATH_F64
        typedef mat3_t<f64> mat3d;
    #endif
#endif
#ifdef MATH_MAT4
    #ifdef MATH_F32
        typedef mat4_t<f32> mat4;
    #endif
    #ifdef MATH_F64
        typedef mat4_t<f64> mat4d;
    #endif
#endif
#ifdef MATH_MAT5
    #ifdef MATH_F32
        typedef mat5_t<f32> mat5;
    #endif
    #ifdef MATH_F64
        typedef mat5_t<f64> mat5d;
    #endif
#endif
#ifdef MATH_COMPLEX
    #ifdef MATH_F32
        typedef complex_t<f32> complex;
    #endif
    #ifdef MATH_F64
        typedef complex_t<f64> complexd;
    #endif
#endif
#ifdef MATH_QUATERNION
    #ifdef MATH_F32
        typedef quat_t<f32> quat;
    #endif
    #ifdef MATH_F64
        typedef quat_t<f64> quatd;
    #endif
#endif

#endif
