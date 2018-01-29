#pragma once

#include "stdafx.h"

class ChunkOrigin;
class MyPlane;


// My own vector classes. The SFML ones don't have a Vec4.


// Floating point comparisons always make me nervous.
const GLfloat EPSILON = 0.0001f;


GLfloat DegreesToRadians(GLfloat degrees);


// A color is technically just a vec-4, but we'll never want to do any matrix-style 
// transformations on these. For the default value, set it to that hideous purple.
class MyColor
{
public:
    MyColor() :
        m_red(1.0f), m_green(0.0f), m_blue(1.0f), m_alpha(1.0f) {}

    MyColor(GLfloat red, GLfloat green, GLfloat blue) :
        m_red(red), m_green(green), m_blue(blue), m_alpha(1.0f) {}

    MyColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) :
        m_red(red), m_green(green), m_blue(blue), m_alpha(alpha) {}

    DEFAULT_COPYING(MyColor)
    DEFAULT_MOVING(MyColor)

    inline GLfloat red()   const { return m_red; }
    inline GLfloat green() const { return m_green; }
    inline GLfloat blue()  const { return m_blue; }
    inline GLfloat alpha() const { return m_alpha; }

private:
    GLfloat m_red;
    GLfloat m_green;
    GLfloat m_blue;
    GLfloat m_alpha;
};

static_assert(sizeof(MyColor) == 16, "MyColor should be 16 bytes.");


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

    DEFAULT_COPYING(MyVec2)
    DEFAULT_MOVING(MyVec2)

    MyVec2 lerp(const MyVec2 &that, float amount) const;
    std::string toString() const;

    inline GLfloat x() const { return m_x; }
    inline GLfloat y() const { return m_y; }

private:
    GLfloat m_x;
    GLfloat m_y;
};

static_assert(sizeof(MyVec2) == 8, "MyVec2 should be 8 bytes.");


MyVec2 FourWayLerp2(
    const MyVec2 &lower_left, const MyVec2 &lower_right,
    const MyVec2 &upper_left, const MyVec2 &upper_right,
    GLfloat amount_x, GLfloat amount_y);


class MyVec4
{
public:
    static GLfloat Dot(const MyVec4 &one, const MyVec4 &two) {
        return (one.m_x * two.m_x) + (one.m_y * two.m_y) + (one.m_z * two.m_z);
    }

    static MyVec4 Cross(const MyVec4 &one, const MyVec4 &two);

    MyVec4() :
        m_x(0.0f), m_y(0.0f), m_z(0.0f), m_w(1.0f) {}

    MyVec4(GLfloat x, GLfloat y, GLfloat z) :
        m_x(x), m_y(y), m_z(z), m_w(1.0f) {}

    MyVec4(GLfloat x, GLfloat y, GLfloat z, GLfloat w) : 
        m_x(x), m_y(y), m_z(z), m_w(w) {}

    DEFAULT_COPYING(MyVec4)
    DEFAULT_MOVING(MyVec4)

    GLfloat length() const {
        return sqrt((m_x * m_x) + (m_y * m_y) + (m_z * m_z));
    }

    GLfloat lengthSquared() const {
        return (m_x * m_x) + (m_y * m_y) + (m_z * m_z);
    }

    bool isNormalLength() const {
        return (abs(length() - 1.0f) < EPSILON);
    }

    MyVec4 times(GLfloat val) const {
        return MyVec4(m_x * val, m_y * val, m_z * val);
    }

    MyVec4 dividedBy(GLfloat val) const {
        return MyVec4(m_x / val, m_y / val, m_z / val);
    }

    MyVec4 plus(const MyVec4 &that) const {
        return MyVec4(m_x + that.m_x, m_y + that.m_y, m_z + that.m_z);
    }

    MyVec4 minus(const MyVec4 &that) const {
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

static_assert(sizeof(MyVec4) == 16, "MyVec4 should be 8 bytes.");


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

    DEFAULT_MOVING(MyMatrix4by4)

    const GLfloat *ptr() const { return &m_v00; }

    MyVec4 times(const MyVec4 &vec);
    MyMatrix4by4 times(const MyMatrix4by4 &that);

    static MyMatrix4by4 Identity();
    static MyMatrix4by4 Translate(const MyVec4 &tvec);
    static MyMatrix4by4 RotateX(GLfloat degrees);
    static MyMatrix4by4 RotateY(GLfloat degrees);
    static MyMatrix4by4 RotateZ(GLfloat degrees);
    static MyMatrix4by4 Frustum(GLfloat angle_of_view, GLfloat aspect_ratio, GLfloat near_plane, GLfloat far_plane);

private:
    FORBID_COPYING(MyMatrix4by4)

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

    DEFAULT_COPYING(MyRay)
    DEFAULT_MOVING(MyRay)

    MyPlane toPlane() const;

    inline const MyVec4 &getStart() const { return m_start; }
    inline const MyVec4 &getDir()   const { return m_dir; }

private:
    MyVec4 m_start;
    MyVec4 m_dir;
};


class MyPlane
{
public:
    MyPlane() :
       m_dist(0.0f) {}

    MyPlane(const MyVec4 &normal, GLfloat dist) :
        m_normal(normal), m_dist(dist) {}
    
    DEFAULT_COPYING(MyPlane)
    DEFAULT_MOVING(MyPlane)

    GLfloat distanceToPoint(const MyVec4 &point) const;

    inline const MyVec4 &getNormal() const { return m_normal; }
    inline GLfloat getDist() const { return m_dist; }

private:
    MyVec4  m_normal;
    GLfloat m_dist;
};


// An oriented bounding box.
// If we ever need a non-oriented one, we'll add that later.
class MyBoundingBox
{
public:
    MyBoundingBox() :
        m_lower(0, 0, 0),
        m_upper(0, 0, 0) {}

    MyBoundingBox(const MyVec4 &lower, const MyVec4 &upper) {
        GLfloat min_x = min(lower.x(), upper.x());
        GLfloat min_y = min(lower.y(), upper.y());
        GLfloat min_z = min(lower.z(), upper.z());

        GLfloat max_x = max(lower.x(), upper.x());
        GLfloat max_y = max(lower.y(), upper.y());
        GLfloat max_z = max(lower.z(), upper.z());

        m_lower = MyVec4(min_x, min_y, min_z);
        m_upper = MyVec4(max_x, max_y, max_z);
    }

    MyBoundingBox translate(const MyVec4 &move) {
        MyMatrix4by4 tr = MyMatrix4by4::Translate(move);
        MyVec4 new_lower = tr.times(m_lower);
        MyVec4 new_upper = tr.times(m_upper);
        return MyBoundingBox(new_lower, new_upper);
    }

    DEFAULT_COPYING(MyBoundingBox)
    DEFAULT_MOVING(MyBoundingBox)

    const MyVec4 &getLower() const { return m_lower; }
    const MyVec4 &getUpper() const { return m_upper; }

    GLfloat minX() const { return m_lower.x(); }
    GLfloat maxX() const { return m_upper.x(); }
    GLfloat minY() const { return m_lower.y(); }
    GLfloat maxY() const { return m_upper.y(); }
    GLfloat minZ() const { return m_lower.z(); }
    GLfloat maxZ() const { return m_upper.z(); }

private:
    MyVec4 m_lower;
    MyVec4 m_upper;
};


// Results from a hit-test.
enum class HitTestType
{
    NONE      = 0,
    PARALLEL  = 1,
    OTHER_WAY = 2,
    BEHIND    = 3,
    SUCCESS   = 4
};


// Hit-test logic, using world coordinates.
HitTestType WorldHitTest(const MyRay &ray, const MyPlane &plane, MyVec4 *pOut_impact, GLfloat *pOut_dist);


// Grid coords.
enum class NudgeType
{
    NONE  = 0,
    EAST  = 1,
    WEST  = 2,
    UP    = 3,
    DOWN  = 4,
    NORTH = 5,
    SOUTH = 6
};


// Global pillar coords. These are valid anywhere.
// We use these for reading from the database.
class GlobalPillar
{
public:
    GlobalPillar() :
        m_x(0), m_z(0) {}

    GlobalPillar::GlobalPillar(int x, int z) :
        m_x(x), m_z(z) {}

    DEFAULT_COPYING(GlobalPillar)
    DEFAULT_MOVING(GlobalPillar)

    bool operator==(const GlobalPillar &that) {
        return ((m_x == that.m_x) &&
                (m_z == that.m_z));
    }

    inline int x() const { return m_x; }
    inline int z() const { return m_z; }

private:
    int m_x;
    int m_z;
};


// We need "less than" since we use "Global Pillar" as a key in "std::map".
bool operator<(const GlobalPillar &one, const GlobalPillar &two);



// Global grid coordinates. These are valid anywhere.
class GlobalGrid
{
public:
    GlobalGrid() :
        m_x(0), m_y(0), m_z(0) {}

    GlobalGrid::GlobalGrid(int x, int y, int z) :
        m_x(x), m_y(y), m_z(z) {}

    DEFAULT_COPYING(GlobalGrid)
    DEFAULT_MOVING(GlobalGrid)

    bool operator==(const GlobalGrid &that) {
        return ((m_x == that.m_x) &&
                (m_y == that.m_y) &&
                (m_z == that.m_z));
    }

    bool isValid() const;

    inline int x() const { return m_x; }
    inline int y() const { return m_y; }
    inline int z() const { return m_z; }

private:
    int m_x;
    int m_y;
    int m_z;
};


// Local pillar coords. There are only valid within one chunk.
// We use these to read values from the database.
class LocalPillar
{
public:
    LocalPillar() :
        m_x(0), m_z(0) {}

    LocalPillar(int x, int z);

    DEFAULT_COPYING(LocalPillar)
    DEFAULT_MOVING(LocalPillar)

    bool operator==(const LocalPillar &that) {
        return ((m_x == that.m_x) &&
                (m_z == that.m_z));
    }

    inline int x() const { return m_x; }
    inline int z() const { return m_z; }

private:
    int m_x;
    int m_z;
};


// Local grid coords. These are only valid within one chunk.
// Yeah, this is the same structure as "global", but I want it 
// to be obvious when we're working in one system or another.
class LocalGrid
{
public:
    LocalGrid() :
        m_x(0), m_y(0), m_z(0) {}

    LocalGrid(int x, int y, int z);

    DEFAULT_COPYING(LocalGrid)
    DEFAULT_MOVING(LocalGrid)

    bool operator==(const LocalGrid &that) {
        return ((m_x == that.m_x) &&
                (m_y == that.m_y) &&
                (m_z == that.m_z));
    }

    inline int x() const { return m_x; }
    inline int y() const { return m_y; }
    inline int z() const { return m_z; }

private:
    int m_x;
    int m_y;
    int m_z;
};


GlobalGrid  WorldPosToGlobalGrid(const MyVec4 &pos, NudgeType nudge_type);
ChunkOrigin GlobalGridToChunkOrigin(const GlobalGrid &coord);
LocalGrid   GlobalGridToLocal(const GlobalGrid &coord, const ChunkOrigin &origin);
LocalPillar GlobalPillarToLocal(const GlobalPillar &pillar, const ChunkOrigin &origin);


// An "eval region", the four values always align along chunk boundaries.
// This should alway result in a 3x3 grid of chunks, or 5x5, etc.
struct EvalRegion
{
public:
    EvalRegion() :
        m_west(0),  m_east(0), 
        m_south(0), m_north(0),
        m_debug_west(0),  m_debug_east(0),
        m_debug_south(0), m_debug_north(0) {}

    EvalRegion(int west, int east, int south, int north);

    DEFAULT_COPYING(EvalRegion)
    DEFAULT_MOVING(EvalRegion)

    bool operator==(const EvalRegion &that) const;
    bool operator!=(const EvalRegion &that) const;

    bool contains(const ChunkOrigin &origin) const;
    EvalRegion expand() const;

    std::string toDebugStr() const;

    inline int west()  const { return m_west; }
    inline int east()  const { return m_east; }
    inline int south() const { return m_south; }
    inline int north() const { return m_north; }

private:
    int m_debug_west;
    int m_debug_east;
    int m_debug_south;
    int m_debug_north;

    int m_west;
    int m_east;
    int m_south;
    int m_north;
};


// Given a chunk origin, figure our our eval region.
EvalRegion WorldPosToEvalRegion(const MyVec4 &pos, int block_count);
