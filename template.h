// Template, UU version
// IGAD/NHTV/UU - Jacco Bikker - 2006-2019

#pragma once

#define TEMPLATE_VERSION "Template_v2019.01"

typedef unsigned char uchar;
typedef unsigned char byte;
typedef int64_t int64;
typedef uint64_t uint64;
typedef unsigned int uint;

#ifdef _MSC_VER
#define ALIGN(x) __declspec(align(x))
#define MALLOC64(x) _aligned_malloc(x, 64)
#define FREE64(x) _aligned_free(x)
#else
#define ALIGN(x) __attribute__((aligned(x)))
#define MALLOC64(x) malloc(x)
#define FREE64(x) free(x)
#define __inline __attribute__((__always_inline__))
#endif

#define clamp(v, a, b) ((std::min)((b), (std::max)((v), (a))))

#define PI 3.14159265358979323846264338327950288419716939937510582097494459072381640628620899862803482534211706798f

#define PREFETCH(x) _mm_prefetch((const char*)(x), _MM_HINT_T0)
#define PREFETCH_ONCE(x) _mm_prefetch((const char*)(x), _MM_HINT_NTA)
#define PREFETCH_WRITE(x) _m_prefetchw((const char*)(x))
#define loadss(mem) _mm_load_ss((const float* const)(mem))
#define broadcastps(ps) _mm_shuffle_ps((ps), (ps), 0)
#define broadcastss(ss) broadcastps(loadss((ss)))

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define likely(expr) (expr)
#define unlikely(expr) (expr)
#else
#define likely(expr) __builtin_expect((expr), true)
#define unlikely(expr) __builtin_expect((expr), false)
#endif

// deterministic rng
static uint seed = 0x12345678;
inline uint random_uint()
{
    seed ^= seed << 13;
    seed ^= seed >> 17;
    seed ^= seed << 5;
    return seed;
}
inline float random_float() { return random_uint() * 2.3283064365387e-10f; }
inline float rand(float range) { return random_float() * range; }

namespace Tmpl8
{

struct timer
{
    typedef std::chrono::high_resolution_clock Clock;
    typedef Clock::time_point TimePoint;
    typedef std::chrono::microseconds MicroSeconds;

    TimePoint start;
    inline timer() : start(get()) {}

    /// Returns the elapsed time, in milliseconds.
    inline float elapsed() const
    {
        auto diff = get() - start;
        auto duration_us = std::chrono::duration_cast<MicroSeconds>(diff);
        return static_cast<float>(duration_us.count()) / 1000.0f;
    }
    static inline TimePoint get()
    {
        return Clock::now();
    }

    inline void reset() { start = get(); }
};

// vectors
class vec2 // adapted from https://github.com/dcow/RayTracer
{
  public:
    union {
        struct
        {
            float x, y;
        };
        float cell[2];
    };
    vec2() = default;
    vec2(float v) : x(v), y(v) {}
    vec2(float x, float y) : x(x), y(y) {}
    vec2 operator-() const { return vec2(-x, -y); }
    vec2 operator+(const vec2& addOperand) const { return vec2(x + addOperand.x, y + addOperand.y); }
    vec2 operator-(const vec2& operand) const { return vec2(x - operand.x, y - operand.y); }
    vec2 operator*(const vec2& operand) const { return vec2(x * operand.x, y * operand.y); }
    vec2 operator*(float operand) const { return vec2(x * operand, y * operand); }
    vec2 operator/(float operand) const { return vec2(x / operand, y / operand); }
    bool operator==(const vec2& operand) const { return (x == operand.x && y == operand.y); }
    bool operator!=(const vec2& operand) const { return !(*this == operand); }
    void operator-=(const vec2& a)
    {
        x -= a.x;
        y -= a.y;
    }
    void operator+=(const vec2& a)
    {
        x += a.x;
        y += a.y;
    }
    void operator*=(const vec2& a)
    {
        x *= a.x;
        y *= a.y;
    }
    void operator*=(float a)
    {
        x *= a;
        y *= a;
    }
    void operator/=(float a)
    {
        x /= a;
        y /= a;
    }

    float& operator[](const int idx) { return cell[idx]; }
    float length() { return sqrtf(x * x + y * y); }
    float sqr_length() { return x * x + y * y; }
    vec2 normalized()
    {
        float r = 1.0f / length();
        return vec2(x * r, y * r);
    }
    void normalize()
    {
        float r = 1.0f / length();
        x *= r;
        y *= r;
    }
    static vec2 normalize(vec2 v) { return v.normalized(); }
    float dot(const vec2& operand) const { return x * operand.x + y * operand.y; }
};

class Rectangle2D
{

  public:
    Rectangle2D() = default;
    Rectangle2D(vec2 min, vec2 max) : min(min), max(max){};

    bool intersects_circle(const vec2& pos, const float radius) const
    {
        float deltaX = pos.x - clamp(pos.x, min.x, max.x);
        float deltaY = pos.y - clamp(pos.y, min.y, max.y);

        return ((deltaX * deltaX) + (deltaY * deltaY)) <= (radius * radius);
    }

    bool intersects(const Rectangle2D& rect) const
    {
        if (min.x > rect.max.x || rect.min.x > max.x) return false;
        if (min.y < rect.max.y || rect.min.y < max.y) return false;

        return true;
    }

    vec2 min;
    vec2 max;
};

class vec3
{
  public:
    union {
        struct
        {
            float x, y, z, dummy;
        };
        float cell[4];
    };
    vec3() = default;
    vec3(float v) : x(v), y(v), z(v) {}
    vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    vec3 operator-() const { return vec3(-x, -y, -z); }
    vec3 operator+(const vec3& addOperand) const { return vec3(x + addOperand.x, y + addOperand.y, z + addOperand.z); }
    vec3 operator-(const vec3& operand) const { return vec3(x - operand.x, y - operand.y, z - operand.z); }
    vec3 operator*(const vec3& operand) const { return vec3(x * operand.x, y * operand.y, z * operand.z); }
    void operator-=(const vec3& a)
    {
        x -= a.x;
        y -= a.y;
        z -= a.z;
    }
    void operator+=(const vec3& a)
    {
        x += a.x;
        y += a.y;
        z += a.z;
    }
    void operator*=(const vec3& a)
    {
        x *= a.x;
        y *= a.y;
        z *= a.z;
    }
    void operator*=(const float a)
    {
        x *= a;
        y *= a;
        z *= a;
    }
    float operator[](const uint& idx) const { return cell[idx]; }
    float& operator[](const uint& idx) { return cell[idx]; }
    float length() const { return sqrtf(x * x + y * y + z * z); }
    float sqr_length() const { return x * x + y * y + z * z; }
    vec3 normalized() const
    {
        float r = 1.0f / length();
        return vec3(x * r, y * r, z * r);
    }
    void normalize()
    {
        float r = 1.0f / length();
        x *= r;
        y *= r;
        z *= r;
    }
    static vec3 normalize(const vec3 v) { return v.normalized(); }
    vec3 cross(const vec3& operand) const
    {
        return vec3(y * operand.z - z * operand.y, z * operand.x - x * operand.z, x * operand.y - y * operand.x);
    }
    float dot(const vec3& operand) const { return x * operand.x + y * operand.y + z * operand.z; }
};

class vec4
{
  public:
    union {
        struct
        {
            float x, y, z, w;
        };
        vec3 xyz;
        float cell[4];
    };
    vec4() = default;
    vec4(float v) : x(v), y(v), z(v), w(v) {}
    vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    vec4(vec3 a, float b) : x(a.x), y(a.y), z(a.z), w(b) {}
    vec4 operator-() const { return vec4(-x, -y, -z, -w); }
    vec4 operator+(const vec4& addOperand) const { return vec4(x + addOperand.x, y + addOperand.y, z + addOperand.z, w + addOperand.w); }
    vec4 operator-(const vec4& operand) const { return vec4(x - operand.x, y - operand.y, z - operand.z, w - operand.w); }
    vec4 operator*(const vec4& operand) const { return vec4(x * operand.x, y * operand.y, z * operand.z, w * operand.w); }
    void operator-=(const vec4& a)
    {
        x -= a.x;
        y -= a.y;
        z -= a.z;
        w -= a.w;
    }
    void operator+=(const vec4& a)
    {
        x += a.x;
        y += a.y;
        z += a.z;
        w += a.w;
    }
    void operator*=(const vec4& a)
    {
        x *= a.x;
        y *= a.y;
        z *= a.z;
        w *= a.w;
    }
    void operator*=(float a)
    {
        x *= a;
        y *= a;
        z *= a;
        w *= a;
    }
    float& operator[](const int idx) { return cell[idx]; }
    float operator[](const uint& idx) const { return cell[idx]; }
    float length() { return sqrtf(x * x + y * y + z * z + w * w); }
    float sqr_length() { return x * x + y * y + z * z + w * w; }
    vec4 normalized()
    {
        float r = 1.0f / length();
        return vec4(x * r, y * r, z * r, w * r);
    }
    void normalize()
    {
        float r = 1.0f / length();
        x *= r;
        y *= r;
        z *= r;
        w *= r;
    }
    static vec4 normalize(vec4 v) { return v.normalized(); }
    float dot(const vec4& operand) const { return x * operand.x + y * operand.y + z * operand.z + w * operand.w; }
};

vec3 normalize(const vec3& v);
vec3 cross(const vec3& a, const vec3& b);
float dot(const vec3& a, const vec3& b);
vec3 operator*(const float& s, const vec3& v);
vec3 operator*(const vec3& v, const float& s);
vec4 operator*(const float& s, const vec4& v);
vec4 operator*(const vec4& v, const float& s);

class uint4
{
  public:
    union {
        struct
        {
            uint x, y, z, w;
        };
        uint cell[4];
    };
    uint4() = default;
    uint4(int v) : x(v), y(v), z(v), w(v) {}
    uint4(int x, int y, int z, int w) : x(x), y(y), z(z), w(w) {}
    uint4 operator+(const uint4& addOperand) const { return uint4(x + addOperand.x, y + addOperand.y, z + addOperand.z, w + addOperand.w); }
    uint4 operator-(const uint4& operand) const { return uint4(x - operand.x, y - operand.y, z - operand.z, w - operand.w); }
    uint4 operator*(const uint4& operand) const { return uint4(x * operand.x, y * operand.y, z * operand.z, w * operand.w); }
    uint4 operator*(uint operand) const { return uint4(x * operand, y * operand, z * operand, w * operand); }
    void operator-=(const uint4& a)
    {
        x -= a.x;
        y -= a.y;
        z -= a.z;
        w -= a.w;
    }
    void operator+=(const uint4& a)
    {
        x += a.x;
        y += a.y;
        z += a.z;
        w += a.w;
    }
    void operator*=(const uint4& a)
    {
        x *= a.x;
        y *= a.y;
        z *= a.z;
        w *= a.w;
    }
    void operator*=(uint a)
    {
        x *= a;
        y *= a;
        z *= a;
        w *= a;
    }
    uint& operator[](const int idx) { return cell[idx]; }
};

class int4
{
  public:
    union {
        struct
        {
            int x, y, z, w;
        };
        int cell[4];
    };
    int4() = default;
    int4(int v) : x(v), y(v), z(v), w(v) {}
    int4(int x, int y, int z, int w) : x(x), y(y), z(z), w(w) {}
    int4 operator-() const { return int4(-x, -y, -z, -w); }
    int4 operator+(const int4& addOperand) const { return int4(x + addOperand.x, y + addOperand.y, z + addOperand.z, w + addOperand.w); }
    int4 operator-(const int4& operand) const { return int4(x - operand.x, y - operand.y, z - operand.z, w - operand.w); }
    int4 operator*(const int4& operand) const { return int4(x * operand.x, y * operand.y, z * operand.z, w * operand.w); }
    int4 operator*(int operand) const { return int4(x * operand, y * operand, z * operand, w * operand); }
    void operator-=(const int4& a)
    {
        x -= a.x;
        y -= a.y;
        z -= a.z;
        w -= a.w;
    }
    void operator+=(const int4& a)
    {
        x += a.x;
        y += a.y;
        z += a.z;
        w += a.w;
    }
    void operator*=(const int4& a)
    {
        x *= a.x;
        y *= a.y;
        z *= a.z;
        w *= a.w;
    }
    void operator*=(int a)
    {
        x *= a;
        y *= a;
        z *= a;
        w *= a;
    }
    int& operator[](const int idx) { return cell[idx]; }
};

class mat4
{
  public:
    mat4()
    {
        memset(cell, 0, 64);
        cell[0] = cell[5] = cell[10] = cell[15] = 1;
    }
    float cell[16];
    float& operator[](const int idx) { return cell[idx]; }
    static mat4 identity()
    {
        mat4 r;
        memset(r.cell, 0, 64);
        r.cell[0] = r.cell[5] = r.cell[10] = r.cell[15] = 1.0f;
        return r;
    }
    static mat4 rotate(vec3 v, float a);
    static mat4 rotatex(const float a);
    static mat4 rotatey(const float a);
    static mat4 rotatez(const float a);
    void invert()
    {
        // from MESA, via http://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix
        const float inv[16] = {
            cell[5] * cell[10] * cell[15] - cell[5] * cell[11] * cell[14] - cell[9] * cell[6] * cell[15] +
                cell[9] * cell[7] * cell[14] + cell[13] * cell[6] * cell[11] - cell[13] * cell[7] * cell[10],
            -cell[1] * cell[10] * cell[15] + cell[1] * cell[11] * cell[14] + cell[9] * cell[2] * cell[15] -
                cell[9] * cell[3] * cell[14] - cell[13] * cell[2] * cell[11] + cell[13] * cell[3] * cell[10],
            cell[1] * cell[6] * cell[15] - cell[1] * cell[7] * cell[14] - cell[5] * cell[2] * cell[15] +
                cell[5] * cell[3] * cell[14] + cell[13] * cell[2] * cell[7] - cell[13] * cell[3] * cell[6],
            -cell[1] * cell[6] * cell[11] + cell[1] * cell[7] * cell[10] + cell[5] * cell[2] * cell[11] -
                cell[5] * cell[3] * cell[10] - cell[9] * cell[2] * cell[7] + cell[9] * cell[3] * cell[6],
            -cell[4] * cell[10] * cell[15] + cell[4] * cell[11] * cell[14] + cell[8] * cell[6] * cell[15] -
                cell[8] * cell[7] * cell[14] - cell[12] * cell[6] * cell[11] + cell[12] * cell[7] * cell[10],
            cell[0] * cell[10] * cell[15] - cell[0] * cell[11] * cell[14] - cell[8] * cell[2] * cell[15] +
                cell[8] * cell[3] * cell[14] + cell[12] * cell[2] * cell[11] - cell[12] * cell[3] * cell[10],
            -cell[0] * cell[6] * cell[15] + cell[0] * cell[7] * cell[14] + cell[4] * cell[2] * cell[15] -
                cell[4] * cell[3] * cell[14] - cell[12] * cell[2] * cell[7] + cell[12] * cell[3] * cell[6],
            cell[0] * cell[6] * cell[11] - cell[0] * cell[7] * cell[10] - cell[4] * cell[2] * cell[11] +
                cell[4] * cell[3] * cell[10] + cell[8] * cell[2] * cell[7] - cell[8] * cell[3] * cell[6],
            cell[4] * cell[9] * cell[15] - cell[4] * cell[11] * cell[13] - cell[8] * cell[5] * cell[15] +
                cell[8] * cell[7] * cell[13] + cell[12] * cell[5] * cell[11] - cell[12] * cell[7] * cell[9],
            -cell[0] * cell[9] * cell[15] + cell[0] * cell[11] * cell[13] + cell[8] * cell[1] * cell[15] -
                cell[8] * cell[3] * cell[13] - cell[12] * cell[1] * cell[11] + cell[12] * cell[3] * cell[9],
            cell[0] * cell[5] * cell[15] - cell[0] * cell[7] * cell[13] - cell[4] * cell[1] * cell[15] +
                cell[4] * cell[3] * cell[13] + cell[12] * cell[1] * cell[7] - cell[12] * cell[3] * cell[5],
            -cell[0] * cell[5] * cell[11] + cell[0] * cell[7] * cell[9] + cell[4] * cell[1] * cell[11] -
                cell[4] * cell[3] * cell[9] - cell[8] * cell[1] * cell[7] + cell[8] * cell[3] * cell[5],
            -cell[4] * cell[9] * cell[14] + cell[4] * cell[10] * cell[13] + cell[8] * cell[5] * cell[14] -
                cell[8] * cell[6] * cell[13] - cell[12] * cell[5] * cell[10] + cell[12] * cell[6] * cell[9],
            cell[0] * cell[9] * cell[14] - cell[0] * cell[10] * cell[13] - cell[8] * cell[1] * cell[14] +
                cell[8] * cell[2] * cell[13] + cell[12] * cell[1] * cell[10] - cell[12] * cell[2] * cell[9],
            -cell[0] * cell[5] * cell[14] + cell[0] * cell[6] * cell[13] + cell[4] * cell[1] * cell[14] -
                cell[4] * cell[2] * cell[13] - cell[12] * cell[1] * cell[6] + cell[12] * cell[2] * cell[5],
            cell[0] * cell[5] * cell[10] - cell[0] * cell[6] * cell[9] - cell[4] * cell[1] * cell[10] +
                cell[4] * cell[2] * cell[9] + cell[8] * cell[1] * cell[6] - cell[8] * cell[2] * cell[5]};
        const float det = cell[0] * inv[0] + cell[1] * inv[4] + cell[2] * inv[8] + cell[3] * inv[12];
        if (det != 0)
        {
            const float invdet = 1.0f / det;
            for (int i = 0; i < 16; i++) cell[i] = inv[i] * invdet;
        }
    }
};

mat4 operator*(const mat4& a, const mat4& b);
bool operator==(const mat4& a, const mat4& b);
bool operator!=(const mat4& a, const mat4& b);
vec4 operator*(const mat4& a, const vec4& b);
vec4 operator*(const vec4& a, const mat4& b);

//From: https://stackoverflow.com/a/1084899
inline bool circle_segment_intersect(vec2 s1, vec2 s2, vec2 cp, float r)
{
    vec2 d = s2 - s1; //Segment direction vector
    vec2 f = s1 - cp; //Circle to segment start vector

    float a = d.dot(d);
    float b = 2 * f.dot(d);
    float c = f.dot(f) - r * r;

    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0)
    {
        // no intersection
        return false;
    }
    else
    {
        // ray didn't totally miss sphere, so there is a solution to the equation.

        discriminant = sqrt(discriminant);

        // either solution may be on or off the ray so need to test both
        // t1 is always the smaller value, because BOTH discriminant and
        // a are nonnegative.
        float t1 = (-b - discriminant) / (2 * a);
        float t2 = (-b + discriminant) / (2 * a);

        // 3x HIT cases:
        //          -o->             --|-->  |            |  --|->
        // Impale(t1 hit,t2 hit), Poke(t1 hit,t2>1), ExitWound(t1<0, t2 hit), 

        // 3x MISS cases:
        //       ->  o                     o ->              | -> |
        // FallShort (t1>1,t2>1), Past (t1<0,t2<0), CompletelyInside(t1<0, t2>1)

        if (t1 >= 0 && t1 <= 1)
        {
            // t1 is the intersection, and it's closer than t2
            // (since t1 uses -b - discriminant)
            // Impale, Poke
            return true;
        }

        // here t1 didn't intersect so we are either started
        // inside the sphere or completely past it
        if (t2 >= 0 && t2 <= 1)
        {
            // ExitWound
            return true;
        }

        // no intn: FallShort, Past, CompletelyInside
        return false;
    }
}

#define BADFLOAT(x) ((*(uint*)&x & 0x7f000000) == 0x7f000000)

}; // namespace Tmpl8
