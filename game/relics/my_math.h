#pragma once

#include "stdafx.h"


class MyChunkOrigin;
class MyPlane;


// My own vector classes. The SFML ones don't have a Vec4.


const GLfloat EPSILON = 0.0001f;


GLfloat DegreesToRadians(GLfloat degrees);


// A color is technically a vec4, but we'll never want to do any matrix-style transformations on these.
class MyColor
{
public:
    MyColor(GLfloat red, GLfloat green, GLfloat blue) :
        m_red(red), m_green(green), m_blue(blue), m_alpha(1.0f) {}
    MyColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) :
        m_red(red), m_green(green), m_blue(blue), m_alpha(alpha) {}

    MyColor &operator=(const MyColor &that);

    inline GLfloat red()   const { return m_red; }
    inline GLfloat green() const { return m_green; }
    inline GLfloat blue()  const { return m_blue; }
    inline GLfloat alpha() const { return m_alpha; }

private:
    // Forbit default ctor.
    MyColor() = delete;

    GLfloat m_red;
    GLfloat m_green;
    GLfloat m_blue;
    GLfloat m_alpha;
};


// Simple colors for debugging.
extern const MyColor MY_WHITE;
extern const MyColor MY_LT_RED;
extern const MyColor MY_LT_GREEN;
extern const MyColor MY_LT_BLUE;
extern const MyColor MY_LT_YELLOW;
extern const MyColor MY_LT_PURPLE;


class MyVec2
{
public:
    MyVec2() :
        m_x(0.0f), m_y(0.0f) {}
    MyVec2(GLfloat x, GLfloat y) :
        m_x(x), m_y(y) {}
    MyVec2(const MyVec2 &that) :
        m_x(that.m_x), m_y(that.m_y) {}

    inline const MyVec2 &MyVec2::operator=(const MyVec2 &that) {
        m_x = that.m_x; m_y = that.m_y; return *this;
    }

    inline const MyVec2 &MyVec2::operator=(const MyVec2 &&that) {
        m_x = that.m_x; m_y = that.m_y; return *this;
    }

    MyVec2 lerp(const MyVec2 &that, float amount) const;
    std::string toString() const;

    inline GLfloat x() const { return m_x; }
    inline GLfloat y() const { return m_y; }

private:
    GLfloat m_x;
    GLfloat m_y;
};


MyVec2 FourWayLerp2(
    const MyVec2 &lower_left, const MyVec2 &lower_right,
    const MyVec2 &upper_left, const MyVec2 &upper_right,
    GLfloat amount_x, GLfloat amount_y);


class MyVec4
{
public:
    inline static GLfloat Dot(const MyVec4 &one, const MyVec4 &two) {
        return (one.m_x * two.m_x) + (one.m_y * two.m_y) + (one.m_z * two.m_z);
    }

    static MyVec4 Cross(const MyVec4 &one, const MyVec4 &two);

    MyVec4() :
        m_x(0.0f), m_y(0.0f), m_z(0.0f), m_w(1.0f) {}
    MyVec4(GLfloat x, GLfloat y, GLfloat z) :
        m_x(x), m_y(y), m_z(z), m_w(1.0f) {}
    MyVec4(GLfloat x, GLfloat y, GLfloat z, GLfloat w) : 
        m_x(x), m_y(y), m_z(z), m_w(w) {}
    MyVec4(const MyVec4 &that) : 
        m_x(that.m_x), m_y(that.m_y), m_z(that.m_z), m_w(that.m_w) {}

    inline const MyVec4 &operator=(const MyVec4 &that) {
        m_x = that.m_x; m_y = that.m_y; m_z = that.m_z; m_w = that.m_w; return *this;
    }

    inline const MyVec4 &operator=(const MyVec4 &&that) {
        m_x = that.m_x; m_y = that.m_y; m_z = that.m_z; m_w = that.m_w; return *this;
    }

    inline GLfloat length() const {
        return sqrt((m_x * m_x) + (m_y * m_y) + (m_z * m_z));
    }

    inline GLfloat lengthSquared() const {
        return (m_x * m_x) + (m_y * m_y) + (m_z * m_z);
    }

    inline bool isNormalLength() const {
        return (abs(length() - 1.0f) < EPSILON);
    }

    inline MyVec4 times(GLfloat val) const {
        return MyVec4(m_x * val, m_y * val, m_z * val);
    }

    inline MyVec4 dividedBy(GLfloat val) const {
        return MyVec4(m_x / val, m_y / val, m_z / val);
    }

    inline MyVec4 plus(const MyVec4 &that) const {
        return MyVec4(m_x + that.m_x, m_y + that.m_y, m_z + that.m_z);
    }

    inline MyVec4 minus(const MyVec4 &that) const {
        return MyVec4(m_x - that.m_x, m_y - that.m_y, m_z - that.m_z);
    }

    MyVec4 lerp(const MyVec4 &that, float amount) const;
    std::string toString() const;

    inline GLfloat x() const { return m_x; }
    inline GLfloat y() const { return m_y; }
    inline GLfloat z() const { return m_z; }
    inline GLfloat w() const { return m_w; }

private:
    GLfloat m_x;
    GLfloat m_y;
    GLfloat m_z;
    GLfloat m_w;
};


MyVec4 FourWayLerp4(
    const MyVec4 &lower_left, const MyVec4 &lower_right,
    const MyVec4 &upper_left, const MyVec4 &upper_right,
    GLfloat amount_x, GLfloat amount_y);


// Handy unit vectors.
extern const MyVec4 VEC4_EASTWARD;
extern const MyVec4 VEC4_WESTWARD;

extern const MyVec4 VEC4_UPWARD;
extern const MyVec4 VEC4_DOWNWARD;

extern const MyVec4 VEC4_NORTHWARD;
extern const MyVec4 VEC4_SOUTHWARD;


class MyMatrix4by4
{
public:
    MyMatrix4by4() :
        m_v00(0.0f), m_v01(0.0f), m_v02(0.0f), m_v03(0.0f),
        m_v10(0.0f), m_v11(0.0f), m_v12(0.0f), m_v13(0.0f),
        m_v20(0.0f), m_v21(0.0f), m_v22(0.0f), m_v23(0.0f),
        m_v30(0.0f), m_v31(0.0f), m_v32(0.0f), m_v33(1.0f) {}

    MyMatrix4by4(
        GLfloat v00, GLfloat v01, GLfloat v02, GLfloat v03,
        GLfloat v10, GLfloat v11, GLfloat v12, GLfloat v13,
        GLfloat v20, GLfloat v21, GLfloat v22, GLfloat v23,
        GLfloat v30, GLfloat v31, GLfloat v32, GLfloat v33) :
        m_v00(v00), m_v01(v01), m_v02(v02), m_v03(v03),
        m_v10(v10), m_v11(v11), m_v12(v12), m_v13(v13),
        m_v20(v20), m_v21(v21), m_v22(v22), m_v23(v23),
        m_v30(v30), m_v31(v31), m_v32(v32), m_v33(v33) {}

    MyMatrix4by4(const MyMatrix4by4 &that) :
        m_v00(that.m_v00), m_v01(that.m_v01), m_v02(that.m_v02), m_v03(that.m_v03),
        m_v10(that.m_v10), m_v11(that.m_v11), m_v12(that.m_v12), m_v13(that.m_v13),
        m_v20(that.m_v20), m_v21(that.m_v21), m_v22(that.m_v22), m_v23(that.m_v23),
        m_v30(that.m_v30), m_v31(that.m_v31), m_v32(that.m_v32), m_v33(that.m_v33) {}

    inline const GLfloat *ptr() const { return &m_v00; }

    const MyMatrix4by4 &operator=(const MyMatrix4by4 &that);

    MyVec4 times(const MyVec4 &vec);
    MyMatrix4by4 times(const MyMatrix4by4 &that);

    static MyMatrix4by4 Identity();
    static MyMatrix4by4 Translate(GLfloat tx, GLfloat ty, GLfloat tz);
    static MyMatrix4by4 RotateX(GLfloat degrees);
    static MyMatrix4by4 RotateY(GLfloat degrees);
    static MyMatrix4by4 RotateZ(GLfloat degrees);
    static MyMatrix4by4 Frustum(GLfloat angle_of_view, GLfloat aspect_ratio, GLfloat near_plane, GLfloat far_plane);

private:
    GLfloat m_v00, m_v01, m_v02, m_v03;
    GLfloat m_v10, m_v11, m_v12, m_v13;
    GLfloat m_v20, m_v21, m_v22, m_v23;
    GLfloat m_v30, m_v31, m_v32, m_v33;
};


class MyRay
{
public:
    MyRay() {}

    MyRay(const MyVec4 &start, const MyVec4 &dir) : 
        m_start(start), m_dir(dir) {
        assert(dir.isNormalLength());
    }

    MyRay(const MyRay &that) : 
        m_start(that.m_start), m_dir(that.m_dir) {}

    const MyRay &operator=(const MyRay &that);

    inline const MyVec4 &getStart() const { return m_start; }
    inline const MyVec4 &getDir()   const { return m_dir; }

    MyPlane toPlane() const;

private:
    MyVec4 m_start;
    MyVec4 m_dir;
};


class MyPlane
{
public:
    MyPlane() :
       m_distance(0.0f) {}
    MyPlane(const MyVec4 &normal, GLfloat dist) :
        m_normal(normal), m_distance(dist) {}
    MyPlane(const MyPlane &that) :
        m_normal(that.m_normal), m_distance(that.m_distance) {}

    MyPlane &operator=(const MyPlane &that);

    inline const MyVec4 &getNormal() const { return m_normal; }
    inline GLfloat getDist() const { return m_distance; }

    GLfloat distanceToPoint(const MyVec4 &point) const;

private:
    MyVec4  m_normal;
    GLfloat m_distance;
};


// Results from a hit-test.
enum HitTestEnum
{
    HITTEST_NONE,
    HITTEST_PARALLEL,
    HITTEST_OTHER_WAY,
    HITTEST_BEHIND,
    HITTEST_SUCCESS
};


// Hit-test logic, using world coordinates.
HitTestEnum WorldHitTest(const MyRay &ray, const MyPlane &plane, MyVec4 *pOut_impact, GLfloat *pOut_dist);


// Grid coords.
enum NudgeEnum
{
    NUDGE_NONE,
    NUDGE_EAST,
    NUDGE_WEST,
    NUDGE_UP,
    NUDGE_DOWN,
    NUDGE_NORTH,
    NUDGE_SOUTH
};


class MyGridCoord
{
public:
    MyGridCoord() :
        m_x(0), m_y(0), m_z(0) {}
    MyGridCoord(int grid_x, int grid_y, int grid_z) :
        m_x(grid_x), m_y(grid_y), m_z(grid_z) {}
    MyGridCoord(const MyGridCoord &that) :
        m_x(that.m_x), m_y(that.m_y), m_z(that.m_z) {}

    const MyGridCoord &operator=(const MyGridCoord &that);

    bool operator==(const MyGridCoord &that);

    inline int x() const { return m_x; }
    inline int y() const { return m_y; }
    inline int z() const { return m_z; }

private:
    int m_x;
    int m_y;
    int m_z;
};


MyGridCoord WorldToGridCoord(const MyVec4 &pos, NudgeEnum nudge_type);


// Chunk eval region.
// The four values always align along chunk boundaries.
// The north and east values are range exclusive (that is, "0 through 9" kind of logic).
struct MyEvalRegion
{
public:
    MyEvalRegion() :
        m_west(0),  m_east(0), 
        m_south(0), m_north(0) {}
    MyEvalRegion(const MyEvalRegion & that) :
        m_west(that.m_west),   m_east(that.m_east),
        m_south(that.m_south), m_north(that.m_north) {}

    MyEvalRegion(int west, int east, int south, int north);

    bool operator==(const MyEvalRegion &that) const;
    bool operator!=(const MyEvalRegion &that) const;

    inline int west()  const { return m_west; }
    inline int east()  const { return m_east; }
    inline int south() const { return m_south; }
    inline int north() const { return m_north; }

    bool containsOrigin(const MyChunkOrigin &origin) const;
    MyEvalRegion expand() const;

private:
    int m_west;
    int m_east;
    int m_south;
    int m_north;
};


// Given a position, figure our our eval region.
MyEvalRegion WorldToEvalRegion(const MyVec4 &pos, GLfloat distance);
