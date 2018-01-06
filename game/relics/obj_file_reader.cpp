
#include "stdafx.h"
#include "obj_file_reader.h"

#include "common_util.h"
#include "format.h"
#include "utils.h"

#include <boost/filesystem.hpp>


/**
* String trimming functions, found here:
*     https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
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


// Less-than operator, since we'll be using Face Groups as a map key.
// Compare the group name first, then the material second.
// Note that ideally, these two names should always go hand-in-hand!
// One group has one material. But, the code to go checking for that 
// subtle and infrequent error would be madness, so I'm not going to try.
bool operator<(const ObjFaceGroup &one, const ObjFaceGroup &two)
{
    const std::string &group1 = one.getGroupName();
    const std::string &mat1   = one.getMaterialName();

    const std::string &group2 = two.getGroupName();
    const std::string &mat2   = two.getMaterialName();

    if      (group1 < group2) { return true; }
    else if (group1 > group2) { return false; }
    else if (mat1   < mat2)   { return true; }
    else if (mat1   > mat2)   { return false; }
    else                      { return false; } // Must be equal.
}


// Parsing errors.
static const std::string PARSING_ERROR = "ERROR!";


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
    PrintDebug(fmt::format("Parsing OBJ file: {}\n", fixed_file_name));

    // TODO: Why can't I use "std::make_unique" here? 
    // The compiler still looks for default ctor.
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
ObjFileReader::ObjFileReader(const std::string &file_name) :
    m_file_name(file_name),
    m_object_name(""),
    m_current_group_name(""),
    m_current_material_name("")
{
}


// We're rolling our own space string tokenizer, since Boost regex is ridiculously slow.
// We deliberately don't save empty strings, and we're careful about trailing spaces too.
// It appears to work, but feel free to test the living crap out of this later.
std::vector<std::string> ObjFileReader::splitStrBySpaces(const std::string &val)
{
    std::vector<std::string> results;
    results.reserve(3);

    std::size_t pos   = 0;
    std::size_t found = val.find_first_of(' ', pos);
    while (found != std::string::npos) {
        if (found != pos) {
            results.push_back(val.substr(pos, found - pos));
        }
        pos   = found + 1;
        found = val.find_first_of(' ', pos);
    }

    if ((pos < val.size()) && (val.at(pos) != ' ')) {
        results.push_back(val.substr(pos));
    }

    return std::move(results);
}


// Same thing for splitting a string by slashes, which we need to parse "face" entries.
// But, empty entries between slashes are fine, and we'll assume our strings are already trimmed.
std::vector<std::string> ObjFileReader::splitStrBySlashes(const std::string &val)
{
    std::vector<std::string> results;
    results.reserve(3);

    std::size_t pos = 0;
    std::size_t found = val.find_first_of('/', pos);
    while (found != std::string::npos) {
        if (found != pos) {
            results.push_back(val.substr(pos, found - pos));
        }
        pos = found + 1;
        found = val.find_first_of(' ', pos);
    }

    if ((pos < val.size()) && (val.at(pos) != ' ')) {
        results.push_back(val.substr(pos));
    }

    return std::move(results);
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

    // Split the line by spaces.
    std::vector<std::string> tokens = splitStrBySpaces(line);

    std::string keyword = tokens[0];

    // Make sure the line has enough tokens.
    int expected;
    if      (keyword == "v")      { expected = 4; }
    else if (keyword == "vn")     { expected = 4; }
    else if (keyword == "vt")     { expected = 3; }
    else if (keyword == "f")      { expected = 4; }
    else if (keyword == "o")      { expected = 2; }
    else if (keyword == "g")      { expected = 2; }
    else if (keyword == "mtlib")  { expected = 2; }
    else if (keyword == "usemtl") { expected = 2; }
    else                          { expected = 1; }

    int what_we_got = static_cast<int>(tokens.size());
    if (what_we_got < expected) {
        PrintDebug(fmt::format(
            "Line {0} has less than the required number of tokens, {1} vs. {2}: {3}\n",
            line_num, expected, tokens.size(), line));
        return false;
    }

    // Deal with all the possible keywords...

    // Positions. Remember to translate to our world's block scale.
    if (keyword == "v") {
        GLfloat x = parseFloat(tokens[1]) * BLOCK_SCALE;
        GLfloat y = parseFloat(tokens[2]) * BLOCK_SCALE;
        GLfloat z = parseFloat(tokens[3]) * BLOCK_SCALE;
        m_positions.emplace_back(x, y, z);
        return true;
    }

    // Surface normals.
    else if (keyword == "vn") {
        GLfloat x = parseFloat(tokens[1]);
        GLfloat y = parseFloat(tokens[2]);
        GLfloat z = parseFloat(tokens[3]);
        m_normals.emplace_back(x, y, z);
        return true;
    }

    // Texture coords.
    else if (keyword == "vt") {
        GLfloat u = parseFloat(tokens[1]);
        GLfloat v = parseFloat(tokens[2]);
        m_tex_coords.emplace_back(u, v);
        return true;
    }

    // Faces.
    else if (keyword == "f") {
        // Look up which face group to add to. Note that total blanks is okay.
        ObjFaceGroup key(m_current_group_name, m_current_material_name);

        // Create the underlying vector if it doesn't exist already.
        if (m_face_groups_map.find(key) == m_face_groups_map.end()) {
            m_face_groups_map[key] = std::make_unique<std::vector<Vertex_PNT>>();
        }

        // Add the faces.
        auto &face_group = m_face_groups_map[key];
        face_group->emplace_back(parseFaceToken(tokens[1]));
        face_group->emplace_back(parseFaceToken(tokens[2]));
        face_group->emplace_back(parseFaceToken(tokens[3]));
        return true;
    }

    // Object name. For right now, saving 
    // this in case it might be useful later.
    else if (keyword == "o") {
        m_object_name = tokens[1];
        return true;
    }

    // Group name.
    else if (keyword == "g") { 
        m_current_group_name = tokens[1];
        return true;
    }

    // Material library.
    else if (keyword == "mtllib") {
        bool result = parseMtllibFile(tokens[1]);
        return result;
    }

    // Material name.
    else if (keyword == "usemtl") {
        const std::string &name = tokens[1];
        assert(m_materials_map.find(name) != m_materials_map.end());
        m_current_material_name = name;
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


// Parse a token from a "face" line. We break this out  since the logic is 
// complicated. Account for blank values, and the "offset by 1" business.
// Side note: It's annoying that std::vector returns an "unsigned int" for size.
// If your vector ever really does grow beyond 2GB entries, you have bigger problems.
Vertex_PNT ObjFileReader::parseFaceToken(const std::string &token)
{
    std::vector<std::string> parts = splitStrBySlashes(token);

    // First, the position.
    MyVec4 position;

    if (parts.size() > 0) {
        if (parts[0] != "") {
            int index = ::atoi(parts[0].c_str());
            if (index > 0) {
                index--;
                int vec_count = static_cast<int>(m_positions.size());
                if (index < vec_count) {
                    position = m_positions.at(index);
                }
            }
        }
    }

    // Second, the surface normal.
    MyVec4 normal;

    if (parts.size() > 1) {
        if (parts[1] != "") {
            int index = ::atoi(parts[1].c_str());
            if (index > 0) {
                index--;
                int vec_count = static_cast<int>(m_normals.size());
                if (index < vec_count) {
                    normal = m_normals.at(index);
                }
            }
        }
    }

    // Third, the tex coord.
    MyVec2 tex_coord;

    if (parts.size() > 2) {
        if (parts[2] != "") {
            int index = ::atoi(parts[2].c_str());
            if (index > 0) {
                index--;
                int vec_count = static_cast<int>(m_tex_coords.size());
                if (index < vec_count) {
                    tex_coord = m_tex_coords.at(index);
                }
            }
        }
    }

    return std::move(Vertex_PNT(position, normal, tex_coord));
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
    PrintDebug(fmt::format("Parsing MTL file: {}\n", real_file_name));
    std::ifstream infile(real_file_name);

    int line_num = 1;
    std::string line;
    while (std::getline(infile, line)) {
        trim_in_place(line);

        // As we parse each line, look for the beginning of a new material name.
        // When we see that, save the current material and start a new one.
        std::string new_name = parseMtllibLine(line_num, line, material.get());
        if (new_name != "") {
            if (new_name == PARSING_ERROR) {
                return false;
            }

            if (material != nullptr) {
                const std::string key = material->getMaterialName();
                m_materials_map[key] = std::move(material);
            }

            material = std::make_unique<ObjMaterial>(new_name);
        }

        line_num++;
    }

    // All done. Save any final material.
    if (material != nullptr) {
        const std::string key = material->getMaterialName();
        m_materials_map[key] = std::move(material);
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

    // Split the line by spaces.
    std::vector<std::string> tokens = splitStrBySpaces(line);
    std::string keyword = tokens[0];

    // Make sure the line has enough tokens.
    int expected;
    if      (keyword == "newmtl") { expected = 2; }
    else if (keyword == "map_Ka") { expected = 2; }
    else if (keyword == "map_Kd") { expected = 2; }
    else                          { expected = 1; }

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
        return tokens[1];
    }

    // Beyond here, it's possible we haven't hit a "newmtl" line yet.
    // If so, there's no material to work with. Account for that.

    // Whichever has the color texmap we want, ambient or diffuse.
    else if ((keyword == "map_Ka") || 
             (keyword == "map_Kd")) {
        if (pOut_material != nullptr) {
            std::string image_file_name = locateRelativeFile(tokens[1]);
            if (image_file_name == "") {
                return PARSING_ERROR;
            }

            if (pOut_material->getTextureImage() == nullptr) {
                std::unique_ptr<sf::Image> image = std::make_unique<sf::Image>();
                if (!image->loadFromFile(image_file_name)) {
                    PrintDebug(fmt::format(
                        "Line {0}: Could not load image {1}.\n",
                        line_num, image_file_name));
                    return PARSING_ERROR;
                }

                pOut_material->setTexture(image_file_name, std::move(image));
                return "";
            }
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
    boost::filesystem::path starter = boost::filesystem::canonical(m_file_name);
    boost::filesystem::path wanted  = starter.parent_path().append(fname);

    if (boost::filesystem::is_regular_file(wanted)) {
        return wanted.string();
    }
    else {
        PrintDebug(fmt::format("Referenced file {0} does not exist!\n", fname));
        return "";
    }
}


// Return a description string.
std::string ObjFileReader::toDescr() const
{
    // Figure out our object dimensions.
    GLfloat min_x =  FLT_MAX;
    GLfloat min_y =  FLT_MAX;
    GLfloat min_z =  FLT_MAX;

    GLfloat max_x = -FLT_MAX;
    GLfloat max_y = -FLT_MAX;
    GLfloat max_z = -FLT_MAX;

    for (const MyVec4 &position : m_positions) {
        GLfloat x = position.x();
        GLfloat y = position.y();
        GLfloat z = position.z();

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
    std::string result = fmt::format("OBJ File: '{}'\n", m_object_name);
    result += fmt::format("    File path = {}\n", m_file_name);

    result += fmt::format(
        "    Dimensions = {0:.3f}m wide, {1:.3f}m deep, {2:.3f}m high\n",
        width, depth, height);

    result += fmt::format("    Position count = {}\n", m_positions.size());
    result += fmt::format("    Normal count = {}\n", m_normals.size());
    result += fmt::format("    Tex Coord count = {}\n", m_tex_coords.size());

    if (m_face_groups_map.size() > 0) {
        int total_faces = 0;
        result += "    Face Groups:\n";
        for (const auto &iter : m_face_groups_map) {
            result += fmt::format(
                "        '{0}'-'{1}' = {2} face vertices\n",
                iter.first.getGroupName(),
                iter.first.getMaterialName(),
                iter.second->size());
            total_faces += iter.second->size();
        }

        result += fmt::format(
            "    Total Face Vertex count = {0} (or, {1} triangles)\n", 
            total_faces, total_faces / 3);
        if ((total_faces % 3) != 0) {
            result += "    ERROR! Number of Face Vertices is NOT divisible by 3!\n";
        }
    }
    else {
        result += "    ERROR! No faces! WTF?!\n";
    }

    if (m_materials_map.size() > 0) {
        result += "    Materials:\n";
        for (const auto &iter : m_materials_map) {
            result += fmt::format(
                "        Name = '{0}', Texture = '{1}'\n", 
                iter.first, iter.second->getTextureFileName());
        }
    }
    else {
        result += "    ERROR! No materials.\n";
    }

    return result;
}
