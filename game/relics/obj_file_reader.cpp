
#include "stdafx.h"
#include "obj_file_reader.h"

#include "common_util.h"
#include "format.h"
#include "utils.h"

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>


/**
 * String trimming functions, found here:
 * https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
 */

// Left trim, in place.
static void left_trim_in_place(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

// Right trim, in place.
static void right_trim_in_place(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// Trim both sides, in place.
static void trim_in_place(std::string &s) {
    left_trim_in_place(s);
    right_trim_in_place(s);
}


// Regular expressions.
static const boost::regex RE_SPACES("\\s+");
static const boost::regex RE_FLOAT("(-)?\\d+(.\\d+)?");
static const boost::regex RE_INTEGER("\\d+");
static const std::string  PARSING_ERROR = "ERROR!";


// Our factory. We hide the constructor in case there's an issue with the file.
std::unique_ptr<ObjFileReader> ObjFileReader::Create(const std::string &OBJ_file_name)
{
    // Make the file name absolute.
    boost::filesystem::path path = boost::filesystem::canonical(OBJ_file_name);

    // Make sure the file exists.
    if (!boost::filesystem::is_regular_file(path)) {
        PrintDebug(fmt::format("OBJ file {0} does not exist!\n", OBJ_file_name));
        assert(false);
        return nullptr;
    }

    std::string fixed_file_name = path.string();

    // Parse the file. If there are any issues, return null.
    PrintDebug(fmt::format("Parsing OBJ file '{}'...\n", fixed_file_name));

    // TODO: Why can't I use "make_unique" here?
    std::unique_ptr<ObjFileReader> result(new ObjFileReader(fixed_file_name));

    std::ifstream infile(fixed_file_name);
    int line_num = 1;
    std::string line;
    while (std::getline(infile, line)) {
        trim_in_place(line);

        if (!result->parseObjLine(line_num, line)) {
            return nullptr;
        }

        line_num++;
    }

    // Otherwise we're okay! Have a great day.
    return std::move(result);
}


// The only allowed constructor.
ObjFileReader::ObjFileReader(const std::string &OBJ_file_name) :
    m_OBJ_file_name(OBJ_file_name),
    m_current_group_name("")
{
}


// Parse an individual line in the OBJ file. For now, we're going to
// assume our file is valid, and not go overboard on the error messages.
// But, return false if we run into anything we genuinely can't make sense of.
bool ObjFileReader::parseObjLine(int line_num, const std::string &line)
{
    // Skip blank lines and comments.
    if ((line == "") || (line.at(0) == '#')) {
        return true;
    }

    // Split the line by spaces. One keyword, then one or more tokens.
    boost::sregex_token_iterator iter(line.begin(), line.end(), RE_SPACES, -1);
    boost::sregex_token_iterator end_thingy;

    std::string keyword = *iter++;

    std::vector<std::string> tokens;
    while (iter != end_thingy) {
        tokens.emplace_back(*iter++);
    }

    // Make sure the line has enough tokens.
    int expected;
    if      (keyword == "v")      { expected = 3; }
    else if (keyword == "vn")     { expected = 3; }
    else if (keyword == "vt")     { expected = 2; }
    else if (keyword == "f")      { expected = 3; }
    else if (keyword == "g")      { expected = 1; }
    else if (keyword == "mtlib")  { expected = 1; }
    else if (keyword == "usemtl") { expected = 1; }
    else                          { expected = 0; }

    int what_we_got = static_cast<int>(tokens.size());
    if (what_we_got < expected) {
        PrintDebug(fmt::format(
            "Line {0} has less than the required number of tokens, {1} vs. {2}: {3}\n",
            line_num, expected, tokens.size(), line));
        return false;
    }

    // Deal with all the possible keywords...

    // Vertex. Remember to translate to our world's block scale.
    if (keyword == "v") {
        GLfloat x = parseFloat(line_num, tokens[0], line) * BLOCK_SCALE;
        GLfloat y = parseFloat(line_num, tokens[1], line) * BLOCK_SCALE;
        GLfloat z = parseFloat(line_num, tokens[2], line) * BLOCK_SCALE;
        m_vertices.emplace_back(x, y, z);
        return true;
    }

    // vertex normal
    else if (keyword == "vn") {
        GLfloat x = parseFloat(line_num, tokens[0], line);
        GLfloat y = parseFloat(line_num, tokens[1], line);
        GLfloat z = parseFloat(line_num, tokens[2], line);
        m_normals.emplace_back(x, y, z);
        return true;
    }

    // texture coord
    else if (keyword == "vt") {
        GLfloat u = parseFloat(line_num, tokens[0], line);
        GLfloat v = parseFloat(line_num, tokens[1], line);
        m_tex_coords.emplace_back(u, v);
        return true;
    }

    // Faces.
    else if (keyword == "f") {
        return true;
    }

    // group name
    else if (keyword == "g") { 
        m_current_group_name = tokens[0];
        return true;
    }

    // material library
    else if (keyword == "mtllib") {
        bool result = parseMtllibFile(tokens[0]);
        return result;
    }

    // material name
    else if (keyword == "usemtl") {
        return true;
    }

    // We just want the vertices, normals, UV coords, and textures,
    // so there are lots of keywords we aren't interested in, such
    // as lines, points, curves, and hints for 3D modeling tools.
    else if (
        (keyword == "vp") ||         // Parameter space vertex
        (keyword == "cstype") ||     // Curve type
        (keyword == "deg") ||        // Degree
        (keyword == "bmat") ||       // Basis matrix
        (keyword == "step") ||       // Step size
        (keyword == "p") ||          // Point
        (keyword == "l") ||          // Line
        (keyword == "curv") ||       // Curve
        (keyword == "curv2") ||      // 2D curve
        (keyword == "surf") ||       // Surface
        (keyword == "parm") ||       // Parameter value
        (keyword == "trim") ||       // Outer trimming loop
        (keyword == "hole") ||       // Inner trimming loop
        (keyword == "scrv") ||       // Special curve
        (keyword == "sp") ||         // Special point
        (keyword == "end") ||        // End of a curve
        (keyword == "con") ||        // Surface connection
        (keyword == "s") ||          // Smoothing group
        (keyword == "mg") ||         // Merging group
        (keyword == "o") ||          // Object name
        (keyword == "bevel") ||      // Bevel interpolation
        (keyword == "c_interp") ||   // Color interpolation
        (keyword == "d_interp") ||   // Dissolve interp
        (keyword == "lod") ||        // Level of detail
        (keyword == "shadow_obj") || // Shadow casting
        (keyword == "trace_obj") ||  // Ray tracing
        (keyword == "ctech") ||      // Curve approximation
        (keyword == "stech")) {      // Surface approximation
        return true;
    }

    // Anything else is a keyword we didn't account for.
    PrintDebug(fmt::format(
        "Line {0} has an unexpected keyword: {1}'\n",
        line_num, line));
    return false;
}


// Parse the material file.
bool ObjFileReader::parseMtllibFile(const std::string &MTL_file_name)
{
    std::string real_file_name = locateRelativeFile(MTL_file_name);
    if (real_file_name == "") {
        return false;
    }

    // We build material objects as we go.
    std::unique_ptr<ObjMaterial> material = nullptr;

    // Parse the file line by line.
    PrintDebug(fmt::format("Parsing MTL file '{}'...\n", real_file_name));
    std::ifstream infile(real_file_name);

    int line_num = 1;
    std::string line;
    while (std::getline(infile, line)) {
        trim_in_place(line);

        // As we parse each line, look for the beginning of a new material.
        std::string new_name = parseMtllibLine(line_num, line, material.get());
        if (new_name != "") {
            if (new_name == PARSING_ERROR) {
                return false;
            }

            if (material != nullptr) {
                m_materials.emplace_back(std::move(material));
            }

            material = std::make_unique<ObjMaterial>(new_name);
        }

        line_num++;
    }

    // All done. Save any final material.
    if (material != nullptr) {
        m_materials.emplace_back(std::move(material));
    }

    return true;
}


// Parse a line in our MTL file.
// Returning a string signals the beginning of a new material.
// Returning a blank means we're still in the current material.
// Ugly, but returning the word "ERROR" means there's an issue with the MTL file.
std::string ObjFileReader::parseMtllibLine(int line_num, const std::string &line, ObjMaterial *pOut_material)
{
    // Skip blanks
    if ((line == "") || (line.at(0) == '#')) {
        return "";
    }

    // Split the line by spaces. One keyword, then one or more tokens.
    boost::sregex_token_iterator iter(line.begin(), line.end(), RE_SPACES, -1);
    boost::sregex_token_iterator end_thingy;

    std::string keyword = *iter++;

    std::vector<std::string> tokens;
    while (iter != end_thingy) {
        tokens.emplace_back(*iter++);
    }

    // Make sure the line has enough tokens.
    int expected;
    if      (keyword == "newmtl") { expected = 1; }
    else if (keyword == "map_Ka") { expected = 1; }
    else if (keyword == "map_Kd") { expected = 1; }
    else                          { expected = 0; }

    int what_we_got = static_cast<int>(tokens.size());
    if (what_we_got < expected) {
        PrintDebug(fmt::format(
            "Line {0} has less than the required number of tokens, {1} vs. {2}: {3}\n",
            line_num, expected, tokens.size(), line));
        return PARSING_ERROR;
    }

    // Deal with all the possible keywords...

    // new material
    if (keyword == "newmtl") {
        return tokens[0];
    }

    // Whichever has the color texmap we want, ambient or diffuse.
    else if ((keyword == "map_Ka") || 
             (keyword == "map_Kd")) {
        std::string img_file_name = locateRelativeFile(tokens[0]);
        if (img_file_name == "") {
            return PARSING_ERROR;
        }

        if (pOut_material->getImage() == nullptr) {
            std::unique_ptr<sf::Image> image = std::make_unique<sf::Image>();
            if (!image->loadFromFile(img_file_name)) {
                PrintDebug(fmt::format(
                    "Line {0}: Could not load image {1}.\n", 
                    line_num, img_file_name));
                return PARSING_ERROR;
            }

            pOut_material->setImage(std::move(image));
            return "";
        }
    }

    // For now, we just want materials with one diffuse texture.
    // so there are tons of keywords we aren't interested in.
    else if (
        (keyword == "Ka") ||       // Ambient color
        (keyword == "Kd") ||       // Diffuse color
        (keyword == "Ke") ||       // Emissive color
        (keyword == "Ks") ||       // Specular color
        (keyword == "Ns") ||       // Specular exponent
        (keyword == "Ni") ||       // Optical density
        (keyword == "Tf") ||       // Transmission filter
        (keyword == "illum") ||    // Illumination model
        (keyword == "d") ||        // Dissolve
        (keyword == "Tr") ||       // Transparency
        (keyword == "map_Ks") ||   // Specular color texmap
        (keyword == "map_Ns") ||   // Specular component texmap
        (keyword == "map_d") ||    // Alpha texmap
        (keyword == "map_bump") || // Bump texmap
        (keyword == "bump") ||     // By any other name
        (keyword == "disp") ||     // Displacement map
        (keyword == "decal") ||    // Stencil decal texmap
        (keyword == "refl") ||     // Reflection map
        (keyword == "blendu") ||   // Horizontal texture blending
        (keyword == "blendv") ||   // Vertical texture blending
        (keyword == "boost") ||    // Boost mip-map sharpness
        (keyword == "o") ||        // Origin offset
        (keyword == "s") ||        // Scale
        (keyword == "t") ||        // Turbulence
        (keyword ==  "texres") ||  // Texture resolution to create
        (keyword == "clamp") ||    // Clamp to 0 to 1 range
        (keyword == "bm") ||       // Bump multiplier
        (keyword == "imfchan") ||  // Which channel to get bump from
        (keyword == "type")) {     // Shape type for reflection textmap
        return "";
    }
        
    // Anything else is a keyword we didn't account for.
    PrintDebug(fmt::format(
        "Line {0} has an unexpected keyword: {1}'\n",
        line_num, line));
    return PARSING_ERROR;
}


// .OBJ files refer to other files relatively.
// Find them and make sure they exist. If they don't, return a blank.
std::string ObjFileReader::locateRelativeFile(const std::string &fname)
{
    boost::filesystem::path starter = boost::filesystem::canonical(m_OBJ_file_name);
    boost::filesystem::path wanted  = starter.parent_path().append(fname);

    if (boost::filesystem::is_regular_file(wanted)) {
        return wanted.string();
    }
    else {
        PrintDebug(fmt::format("Referenced file {0} does not exist!\n", fname));
        return "";
    }
}


// Parse a floating point value.
GLfloat ObjFileReader::parseFloat(int line_num, const std::string &val, const std::string &line)
{
    if (boost::regex_match(val, RE_FLOAT)) {
        double result = ::atof(val.c_str());
        return static_cast<GLfloat>(result);
    }
    else {
        PrintDebug(fmt::format(
            "Line {0} has an invalid floating point value of '{1}'\n",
            line_num, val));
        return 0.0f;
    }
}


// Parse an integer value.
int ObjFileReader::parseInt(int line_num, const std::string &val, const std::string &line)
{
    if (boost::regex_match(val, RE_INTEGER)) {
        int result = ::atoi(val.c_str());
        return result;
    }
    else {
        PrintDebug(fmt::format(
            "Line {0} has an invalid integer value of '{1}'\n",
            line_num, val));
        return 0;
    }
}


// Return a description string.
// TODO: Add more here...
std::string ObjFileReader::toDescr() const
{
    // Figure out our object dimensions.
    GLfloat min_x =  FLT_MAX;
    GLfloat min_y =  FLT_MAX;
    GLfloat min_z =  FLT_MAX;

    GLfloat max_x = -FLT_MAX;
    GLfloat max_y = -FLT_MAX;
    GLfloat max_z = -FLT_MAX;

    for (const MyVec4 &vert : m_vertices) {
        GLfloat x = vert.x();
        GLfloat y = vert.y();
        GLfloat z = vert.z();

        if (min_x > x) { min_x = x; }
        if (min_y > y) { min_y = y; }
        if (min_z > z) { min_z = z; }

        if (max_x < x) { max_x = x; }
        if (max_y < y) { max_y = y; }
        if (max_z < z) { max_z = z; }
    }

    GLfloat width  = (max_x - min_x) / BLOCK_SCALE;
    GLfloat height = (max_y - min_y) / BLOCK_SCALE;
    GLfloat depth  = (max_z - min_z) / BLOCK_SCALE;

    // Compose our description.
    std::string result = fmt::format("OBJ File: {}\n", m_OBJ_file_name);

    result += fmt::format(
        "    Dimensions = {0:.3f}m wide by {1:.3f}m deep x {2:.3f}m high\n",
        width, depth, height);

    result += fmt::format("    Vertex count = {}\n", m_vertices.size());
    result += fmt::format("    Normal count = {}\n", m_normals.size());

    return result;
}
