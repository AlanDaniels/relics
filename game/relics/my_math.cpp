
#include "stdafx.h"
#include "my_math.h"
#include "common_util.h"

#include "chunk.h"
#include "config.h"
#include "format.h"
#include "utils.h"


// TODO: Seriously considering my own sin and cosine, for consistent results.
static const GLfloat MY_PI = 3.1415926535897932f;


// Convert to radians.
GLfloat DegreesToRadians(GLfloat degrees)
{
    return (degrees / 180.0f) * MY_PI;
}


// Simple colors for debugging.
const MyColor MY_WHITE(1.0f, 1.0f, 1.0f);
const MyColor MY_LT_RED(1.0f, 0.3f, 0.3f);
const MyColor MY_LT_GREEN(0.3f, 1.0f, 0.3f);
const MyColor MY_LT_BLUE(0.3f, 0.3f, 1.0f);
const MyColor MY_LT_YELLOW(1.0f, 1.0f, 0.3f);
const MyColor MY_LT_PURPLE(1.0f, 0.3f, 1.0f);


// Do a linear interpolation between two Vec2 values.
MyVec2 MyVec2::lerp(const MyVec2 &that, float amount) const
{
    if (amount < 0.0f) {
        return *this;
    }
    else if (amount > 1.0f) {
        return that;
    }
    else {
        return MyVec2(
            m_x + (amount * (that.m_x - m_x)),
            m_y + (amount * (that.m_y - m_y)));
    }
}


// Convert to a string.
std::string MyVec2::toString() const
{
    return fmt::format("{0:0.2f} {1:0.2f}", m_x, m_y);
}


// Lerp both X and Y between four Vec2s.
MyVec2 FourWayLerp2(
    const MyVec2 &lower_left, const MyVec2 &lower_right,
    const MyVec2 &upper_left, const MyVec2 &upper_right,
    GLfloat amount_x, GLfloat amount_y)
{
    MyVec2 lower_mid = lower_left.lerp(lower_right, amount_x);
    MyVec2 upper_mid = upper_left.lerp(upper_right, amount_x);
    return lower_mid.lerp(upper_mid, amount_y);
}




// Handy unit vectors. Note that I'm using a left-handed coordinate system.
const MyVec4 VEC4_EASTWARD(1.0f, 0.0f, 0.0f);
const MyVec4 VEC4_WESTWARD(-1.0f, 0.0f, 0.0f);

const MyVec4 VEC4_UPWARD(0.0f, 1.0f, 0.0f);
const MyVec4 VEC4_DOWNWARD(0.0f, -1.0f, 0.0f);

const MyVec4 VEC4_NORTHWARD(0.0f, 0.0f, 1.0f);
const MyVec4 VEC4_SOUTHWARD(0.0f, 0.0f, -1.0f);


// Vec4 cross product.
MyVec4 MyVec4::Cross(const MyVec4 &one, const MyVec4 &two)
{
    GLfloat x = (one.m_y * two.m_z) - (one.m_z * two.m_y);
    GLfloat y = (one.m_z * two.m_x) - (one.m_x * two.m_z);
    GLfloat z = (one.m_x * two.m_y) - (one.m_y * two.m_x);
    return MyVec4(x, y, z);
}


// Do a linear interpolation between two Vec4 values.
MyVec4 MyVec4::lerp(const MyVec4 &that, float amount) const
{
    if (amount < 0.0f) {
        return *this;
    }
    else if (amount > 1.0f) {
        return that;
    }
    else {
        return MyVec4(
            m_x + (amount * (that.m_x - m_x)),
            m_y + (amount * (that.m_y - m_y)),
            m_z + (amount * (that.m_z - m_z)),
            1.0f);
    }
}


// Convert to a string.
std::string MyVec4::toString() const
{
    return fmt::format("{0:.0f} {1:.0f} {2:.0f}", m_x, m_y, m_z);
}


// Lerp both X and Y between four Vec4s.
MyVec4 FourWayLerp4(
    const MyVec4 &lower_left, const MyVec4 &lower_right,
    const MyVec4 &upper_left, const MyVec4 &upper_right,
    GLfloat amount_x, GLfloat amount_y)
{
    MyVec4 lower_mid = lower_left.lerp(lower_right, amount_x);
    MyVec4 upper_mid = upper_left.lerp(upper_right, amount_x);
    return lower_mid.lerp(upper_mid, amount_y);
}


// Matrix mulitplication for a vector.
MyVec4 MyMatrix4by4::times(const MyVec4 &vec)
{
    GLfloat x = (m_v00 * vec.x()) + (m_v01 * vec.y()) + (m_v02 * vec.z()) + (m_v03 * vec.w());
    GLfloat y = (m_v10 * vec.x()) + (m_v11 * vec.y()) + (m_v12 * vec.z()) + (m_v13 * vec.w());
    GLfloat z = (m_v20 * vec.x()) + (m_v21 * vec.y()) + (m_v22 * vec.z()) + (m_v23 * vec.w());
    GLfloat w = (m_v30 * vec.x()) + (m_v31 * vec.y()) + (m_v32 * vec.z()) + (m_v33 * vec.w());
    return MyVec4(x, y, z, w);
}


// Matrix mulitplication for another matrix.
MyMatrix4by4 MyMatrix4by4::times(const MyMatrix4by4 &that)
{
    GLfloat v00 = (m_v00 * that.m_v00) + (m_v01 * that.m_v10) + (m_v02 * that.m_v20) + (m_v03 * that.m_v30);
    GLfloat v01 = (m_v00 * that.m_v01) + (m_v01 * that.m_v11) + (m_v02 * that.m_v21) + (m_v03 * that.m_v31);
    GLfloat v02 = (m_v00 * that.m_v02) + (m_v01 * that.m_v12) + (m_v02 * that.m_v22) + (m_v03 * that.m_v32);
    GLfloat v03 = (m_v00 * that.m_v03) + (m_v01 * that.m_v13) + (m_v02 * that.m_v23) + (m_v03 * that.m_v33);

    GLfloat v10 = (m_v10 * that.m_v00) + (m_v11 * that.m_v10) + (m_v12 * that.m_v20) + (m_v13 * that.m_v30);
    GLfloat v11 = (m_v10 * that.m_v01) + (m_v11 * that.m_v11) + (m_v12 * that.m_v21) + (m_v13 * that.m_v31);
    GLfloat v12 = (m_v10 * that.m_v02) + (m_v11 * that.m_v12) + (m_v12 * that.m_v22) + (m_v13 * that.m_v32);
    GLfloat v13 = (m_v10 * that.m_v03) + (m_v11 * that.m_v13) + (m_v12 * that.m_v23) + (m_v13 * that.m_v33);

    GLfloat v20 = (m_v20 * that.m_v00) + (m_v21 * that.m_v10) + (m_v22 * that.m_v20) + (m_v23 * that.m_v30);
    GLfloat v21 = (m_v20 * that.m_v01) + (m_v21 * that.m_v11) + (m_v22 * that.m_v21) + (m_v23 * that.m_v31);
    GLfloat v22 = (m_v20 * that.m_v02) + (m_v21 * that.m_v12) + (m_v22 * that.m_v22) + (m_v23 * that.m_v32);
    GLfloat v23 = (m_v20 * that.m_v03) + (m_v21 * that.m_v13) + (m_v22 * that.m_v23) + (m_v23 * that.m_v33);

    GLfloat v30 = (m_v30 * that.m_v00) + (m_v31 * that.m_v10) + (m_v32 * that.m_v20) + (m_v33 * that.m_v30);
    GLfloat v31 = (m_v30 * that.m_v01) + (m_v31 * that.m_v11) + (m_v32 * that.m_v21) + (m_v33 * that.m_v31);
    GLfloat v32 = (m_v30 * that.m_v02) + (m_v31 * that.m_v12) + (m_v32 * that.m_v22) + (m_v33 * that.m_v32);
    GLfloat v33 = (m_v30 * that.m_v03) + (m_v31 * that.m_v13) + (m_v32 * that.m_v23) + (m_v33 * that.m_v33);

    return MyMatrix4by4(
        v00, v01, v02, v03,
        v10, v11, v12, v13,
        v20, v21, v22, v23,
        v30, v31, v32, v33);
}


// Build an Identity matrix
MyMatrix4by4 MyMatrix4by4::Identity()
{
    return MyMatrix4by4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
}


// Build a translation matrix.
MyMatrix4by4 MyMatrix4by4::Translate(const MyVec4 &tvec)
{
    return MyMatrix4by4(
        1.0f, 0.0f, 0.0f, tvec.x(),
        0.0f, 1.0f, 0.0f, tvec.y(),
        0.0f, 0.0f, 1.0f, tvec.z(),
        0.0f, 0.0f, 0.0f, 1.0f);
}


// Build a rotation X matrix.
MyMatrix4by4 MyMatrix4by4::RotateX(GLfloat degrees)
{
    GLfloat rads = DegreesToRadians(degrees);
    GLfloat sin_val = sin(rads);
    GLfloat cos_val = cos(rads);

    return MyMatrix4by4(
        1.0f,    0.0f,     0.0f, 0.0f,
        0.0f, cos_val, -sin_val, 0.0f,
        0.0f, sin_val,  cos_val, 0.0f,
        0.0f,    0.0f,     0.0f, 1.0f);
}


// Build a rotation Y matrix.
MyMatrix4by4 MyMatrix4by4::RotateY(GLfloat degrees)
{ 
    GLfloat rads = DegreesToRadians(degrees);
    GLfloat sin_val = sin(rads);
    GLfloat cos_val = cos(rads);

    return MyMatrix4by4(
         cos_val, 0.0f, sin_val, 0.0f,
            0.0f, 1.0f,    0.0f, 0.0f,
        -sin_val, 0.0f, cos_val, 0.0f,
            0.0f, 0.0f,    0.0f, 1.0f);
}


// Build a rotation Z matrix.
MyMatrix4by4 MyMatrix4by4::RotateZ(GLfloat degrees)
{
    GLfloat rads = DegreesToRadians(degrees);
    GLfloat sin_val = sin(rads);
    GLfloat cos_val = cos(rads);

    return MyMatrix4by4(
        cos_val, -sin_val, 0.0f, 0.0f,
        sin_val,  cos_val, 0.0f, 0.0f,
        0.0f,        0.0f, 1.0f, 0.0f,
        0.0f,        0.0f, 0.0f, 1.0f);
}


// Build a model-view matrix.
MyMatrix4by4 MyMatrix4by4::Frustum(
    GLfloat angle_of_view, GLfloat aspect_ratio, GLfloat near_plane, GLfloat far_plane) 
{
    GLfloat tangent = tan(angle_of_view);

    return MyMatrix4by4(
        1.0f / tangent, 0.0f,         0.0f, 0.0f,
        0.0f, aspect_ratio / tangent, 0.0f, 0.0f,
        0.0f, 0.0f,        (far_plane + near_plane) / (far_plane - near_plane), 1.0f,
        0.0f, 0.0f, -2.0f * far_plane * near_plane  / (far_plane - near_plane), 0.0f);
}


// Convert a ray to a plane.
// The negative value for the distance is correct.
MyPlane MyRay::toPlane() const
{
    GLfloat dot = (
        (m_dir.x() * m_start.x()) +
        (m_dir.y() * m_start.y()) +
        (m_dir.z() * m_start.z()));
    return MyPlane(m_dir, dot);
}


// Distance from a point to a plane.
GLfloat MyPlane::distanceToPoint(const MyVec4 &point) const
{
    return (point.x() * m_normal.x()) +
           (point.y() * m_normal.y()) +
           (point.z() * m_normal.z()) - m_dist;
}


// Hit-test logic, using world coordinates.
// If we're successful, then fill in the ouput params.
HitTestType WorldHitTest(const MyRay &ray, const MyPlane &plane, MyVec4 *pOut_impact, GLfloat *pOut_distance)
{
    const MyVec4 &normal = plane.getNormal();

    // Calc the distance from our start point to the plane.
    GLfloat denominator = MyVec4::Dot(normal, ray.getDir());
    if (abs(denominator) < EPSILON) {
        return HitTestType::PARALLEL;
    }

    GLfloat numerator = MyVec4::Dot(ray.getStart(), normal) - plane.getDist();
    GLfloat distance  = -numerator / denominator;

    // The plane is "behind" the ray, so the ray will never hit it.
    if (distance < 0.0f) {
        return HitTestType::OTHER_WAY;
    }

    // Get a point on the plane.
    MyVec4 impact = ray.getStart().plus(ray.getDir().times(distance));

    // Check that the ray's position is above the plane.
    MyVec4 reverse = ray.getStart().minus(normal);
    double dot = MyVec4::Dot(reverse, normal) - plane.getDist();
    if (dot < 0) {
        return HitTestType::BEHIND;
    }
    else {
        *pOut_impact   = impact;
        *pOut_distance = distance;
        return HitTestType::SUCCESS;
    }
}


// Return true if a global grid coord has a valid Y-value.
bool GlobalGrid::isValid() const {
    return ((m_y >= 0) && (m_y < CHUNK_HEIGHT));
}



// We need "less than" since we use "Global Pillar" as a key in "std::map".
bool operator<(const GlobalPillar &one, const GlobalPillar &two)
{
    if      (one.x() < two.x()) { return true; }
    else if (one.x() > two.x()) { return false; }
    else if (one.z() < two.z()) { return true; }
    else if (one.z() > two.z()) { return false; }

    // Must be equal.
    else { 
        return false; 
    }
}


// Local Pillar ctor.
LocalPillar::LocalPillar(int x, int z) : 
    m_x(x), m_z(z) 
{
    assert((x >= 0) && (x < CHUNK_WIDTH));
    assert((z >= 0) && (z < CHUNK_WIDTH));
}


// Local Grid ctor.
LocalGrid::LocalGrid(int x, int y, int z) : 
    m_x(x), m_y(y), m_z(z) 
{
    assert((x >= 0) && (x < CHUNK_WIDTH));
    assert((y >= 0) && (y < CHUNK_HEIGHT));
    assert((z >= 0) && (z < CHUNK_WIDTH));
}


// Convert a world position into a grid coord.
// The nudge factor is so we can "burrow" into a block a bit, when we do a
// hit-test against a block face, so that we don't hit strange edge cases in the math.
GlobalGrid WorldPosToGlobalGrid(const MyVec4 &pos, NudgeType nudge)
{
    float fixed_x = pos.x();
    float fixed_y = pos.y();
    float fixed_z = pos.z();

    switch (nudge) {
    case NudgeType::NONE: break;
    case NudgeType::EAST:  fixed_x += NUDGE_AMOUNT; break;
    case NudgeType::WEST:  fixed_x -= NUDGE_AMOUNT; break;
    case NudgeType::UP:    fixed_y += NUDGE_AMOUNT; break;
    case NudgeType::DOWN:  fixed_y -= NUDGE_AMOUNT; break;
    case NudgeType::NORTH: fixed_z += NUDGE_AMOUNT; break;
    case NudgeType::SOUTH: fixed_z -= NUDGE_AMOUNT; break;
    default: 
        PrintTheImpossible(__FILE__, __LINE__, static_cast<int>(nudge));
    }

    int x = static_cast<int>(floor(fixed_x / BLOCK_SCALE));
    int y = static_cast<int>(floor(fixed_y / BLOCK_SCALE));
    int z = static_cast<int>(floor(fixed_z / BLOCK_SCALE));
    return GlobalGrid(x, y, z);
}


// Given a global grid coord, find the chunk origin where this coord would be found.
ChunkOrigin GlobalGridToChunkOrigin(const GlobalGrid &coord)
{
    int x = RoundDownInt(coord.x(), CHUNK_WIDTH);
    int z = RoundDownInt(coord.z(), CHUNK_WIDTH);
    return ChunkOrigin(x, z);
}


// Convert a global grid coord to a local one.
// It is up to *you* to make sure you're calling this for the right chunk origin.
// Also, to make sure the Y value for height is clamped to valid values.
LocalGrid GlobalGridToLocal(const GlobalGrid &coord, const ChunkOrigin &origin)
{
    int x = coord.x() - origin.x();
    int y = coord.y();
    int z = coord.z() - origin.z();
    return LocalGrid(x, y, z);
}


// Convert a global pillar to a local one.
LocalPillar GlobalPillarToLocal(const GlobalPillar &pillar, const ChunkOrigin &origin)
{
    int x = pillar.x() - origin.x();
    int z = pillar.z() - origin.z();
    return LocalPillar(x, z);
}


// Chunk eval region ctor.
EvalRegion::EvalRegion(int west, int east, int south, int north) :
    m_west(west),   m_east(east),
    m_south(south), m_north(north)
{
    // Maxes are greater than or equal to mins.
    assert(west  <= (east  + CHUNK_WIDTH));
    assert(south <= (north + CHUNK_WIDTH));

    // Make sure it always aligns along origons.
    assert((m_west  % CHUNK_WIDTH) == 0);
    assert((m_east  % CHUNK_WIDTH) == 0);
    assert((m_south % CHUNK_WIDTH) == 0);
    assert((m_north % CHUNK_WIDTH) == 0);

    m_debug_west  = west  / CHUNK_WIDTH;
    m_debug_east  = east  / CHUNK_WIDTH;
    m_debug_south = south / CHUNK_WIDTH;
    m_debug_north = north / CHUNK_WIDTH;
}


// See if two eval regions are the same.
bool EvalRegion::operator==(const EvalRegion &that) const
{
    return (
        (m_west  == that.m_west)  &&
        (m_east  == that.m_east)  &&
        (m_south == that.m_south) &&
        (m_north == that.m_north));
}


// See if two eval regions are *not* the same.
bool EvalRegion::operator!=(const EvalRegion &that) const
{
    return (
        (m_west  != that.m_west)  ||
        (m_east  != that.m_east)  ||
        (m_south != that.m_south) ||
        (m_north != that.m_north));
}


// Return true if an eval region contains a chunk origin.
bool EvalRegion::contains(const ChunkOrigin &origin) const
{
    int x = origin.x();
    int z = origin.z();
    return (
        (m_west  <= x) && (x <= m_east) &&
        (m_south <= z) && (z <= m_north));
}


// Grow the eval region by one chunk size.
EvalRegion EvalRegion::expand() const 
{
    return EvalRegion(
        m_west  - CHUNK_WIDTH,
        m_east  + CHUNK_WIDTH,
        m_south - CHUNK_WIDTH,
        m_north + CHUNK_WIDTH);
}


// Return a debug version of an eval region.
std::string EvalRegion::toDebugStr() const
{
    return fmt::format(
        "w={0}, e={1}, s={2}, n={3}",
        m_debug_west, m_debug_east, m_debug_south, m_debug_north);
}


// Given a position and aneval block count, calc the "eval region".
// That is, what's the min and max for X and Z, giving us all the chunks
// that fall within this range. This should always be a 3x3 grid, 5x5, etc.
// Note that this is range-inclusive (ex: -2 to +2), etc.
EvalRegion WorldPosToEvalRegion(const MyVec4 &pos, int block_count)
{
    ChunkOrigin origin = WorldToChunkOrigin(pos);

    int east  = origin.x() + (CHUNK_WIDTH * block_count);
    int west  = origin.x() - (CHUNK_WIDTH * block_count);
    int north = origin.z() + (CHUNK_WIDTH * block_count);
    int south = origin.z() - (CHUNK_WIDTH * block_count);

    return EvalRegion(west, east, south, north);
}
