
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
static boost::regex re_spaces("\\s+");
static boost::regex re_float("(-)?\\d+(.\\d+)?");
static boost::regex re_integer("\\d+");


// The only allowed constructor.
ObjFileReader::ObjFileReader(const std::string &fake_OBJ_fname) :
    m_object_name(""),
    m_current_group_name("")
{
    boost::filesystem::path path_OBJ = boost::filesystem::canonical(fake_OBJ_fname);
    if (!boost::filesystem::is_regular_file(path_OBJ)) {
        PrintDebug(fmt::format("OBJ file {0} does not exist!\n", fake_OBJ_fname));
        assert(false);
        return;
    }

    m_OBJ_fname = path_OBJ.string();
    PrintDebug(fmt::format("Parsing OBJ file '{}'...\n", m_OBJ_fname));
    std::ifstream infile(m_OBJ_fname);

    int line_num = 1;
    std::string line;
    while (std::getline(infile, line)) {
        trim_in_place(line);
        parseObjLine(line_num, line);
        line_num++;
    }
}


// Parse an individual line in the OBJ file. For now, we're going to
// assume our file is valid, and not go overboard on the error messages.
void ObjFileReader::parseObjLine(int line_num, const std::string &line)
{
    // Skip blank lines and comments.
    if ((line == "") || (line.at(0) == '#')) {
        return;
    }

    // Split the line by spaces. One keyword, then one or more tokens.
    boost::sregex_token_iterator iter(line.begin(), line.end(), re_spaces, -1);
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
    else if (keyword == "o")      { expected = 1; }
    else if (keyword == "g")      { expected = 1; }
    else if (keyword == "s")      { expected = 1; }
    else if (keyword == "mtlib")  { expected = 1; }
    else if (keyword == "usemtl") { expected = 1; }
    else                          { expected = 0; }

    int what_we_got = static_cast<int>(tokens.size());
    if (what_we_got < expected) {
        PrintDebug(fmt::format(
            "Line {0} has less than the required number of tokens, {1} vs. {2}: {3}\n",
            line_num, expected, tokens.size(), line));
        return;
    }

    // Deal with all the possible keywords...

    // Vertex. Remember to translate to our block scale.
    if (keyword == "v") {
        GLfloat x = parseFloat(line_num, tokens[0], line) * BLOCK_SCALE;
        GLfloat y = parseFloat(line_num, tokens[1], line) * BLOCK_SCALE;
        GLfloat z = parseFloat(line_num, tokens[2], line) * BLOCK_SCALE;
        m_vertices.emplace_back(x, y, z);
        return;
    }

    // vertex normal
    else if (keyword == "vn") {
        GLfloat x = parseFloat(line_num, tokens[0], line);
        GLfloat y = parseFloat(line_num, tokens[1], line);
        GLfloat z = parseFloat(line_num, tokens[2], line);
        m_normals.emplace_back(x, y, z);
        return;
    }

    // texture coord
    else if (keyword == "vt") {
        GLfloat u = parseFloat(line_num, tokens[0], line);
        GLfloat v = parseFloat(line_num, tokens[1], line);
        m_tex_coords.emplace_back(u, v);
        return;
    }

    // Faces.
    else if (keyword == "f") {
        return;
    }

    // object name
    if (keyword == "o") {
        m_object_name = tokens[0];
        return;
    }

    // group name
    else if (keyword == "g") { 
        m_current_group_name = tokens[0];
        return;
    }

    // smoothing group
    else if (keyword == "s") { 
        const std::string &value = tokens[0];
        return;
    }

    // material library
    else if (keyword == "mtllib") {
        parseMtllibFile(tokens[0]);
        return;
    }

    // material name
    else if (keyword == "usemtl") {
        return;
    }

    // Ignore any other keywords.
    else {
        PrintDebug(fmt::format(
            "Line {0} has a keyword we ignore: {1}'\n",
            line_num, line));
        return;
    }
}


// Parse the material file.
void ObjFileReader::parseMtllibFile(const std::string &partial_MTL_fname)
{
    // The path to the MTL file is relative, so locate it.
    boost::filesystem::path path_OBJ = boost::filesystem::canonical(m_OBJ_fname);
    boost::filesystem::path path_MTL = path_OBJ.parent_path().append(partial_MTL_fname);

    if (!boost::filesystem::is_regular_file(path_MTL)) {
        PrintDebug(fmt::format("MTL file {0} does not exist!\n", partial_MTL_fname));
        assert(false);
        return;
    }

    // We build material objects as we go.
    std::unique_ptr<ObjMaterial> material = nullptr;

    // Parse the file line by line.
    std::string real_MTL_fname = path_MTL.string();
    PrintDebug(fmt::format("Parsing MTL file '{}'...\n", real_MTL_fname));
    std::ifstream infile(real_MTL_fname);

    int line_num = 1;
    std::string line;
    while (std::getline(infile, line)) {
        trim_in_place(line);

        // As we parse each line, look for the beginning of a new material.
        std::string new_name = parseMtllibLine(line_num, line, material.get());
        if (new_name != "") {
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
}


// Parse a line in our MTL file.
// Returning a string signals the beginning of a new material.
std::string ObjFileReader::parseMtllibLine(int line_num, const std::string &line, ObjMaterial *pOut_material)
{
    // Skip blanks
    if ((line == "") || (line.at(0) == '#')) {
        return "";
    }

    // Split the line by spaces. One keyword, then one or more tokens.
    boost::sregex_token_iterator iter(line.begin(), line.end(), re_spaces, -1);
    boost::sregex_token_iterator end_thingy;

    std::string keyword = *iter++;

    std::vector<std::string> tokens;
    while (iter != end_thingy) {
        tokens.emplace_back(*iter++);
    }

    // Make sure the line has enough tokens.
    int expected;
    if      (keyword == "newmtl") { expected = 1; }
    else if (keyword == "Ka")     { expected = 3; }
    else if (keyword == "Kd")     { expected = 3; }
    else if (keyword == "Ke")     { expected = 3; }
    else if (keyword == "Ks")     { expected = 3; }
    else if (keyword == "Ns")     { expected = 1; }
    else if (keyword == "d")      { expected = 1; }
    else if (keyword == "Tr")     { expected = 1; }
    else if (keyword == "Ni")     { expected = 1; }
    else { expected = 0; }

    int what_we_got = static_cast<int>(tokens.size());
    if (what_we_got < expected) {
        PrintDebug(fmt::format(
            "Line {0} has less than the required number of tokens, {1} vs. {2}: {3}\n",
            line_num, expected, tokens.size(), line));
        return "";
    }

    // Deal with all the possible keywords...

    // new material
    if (keyword == "newmtl") {
        return tokens[0];
    }

    // ambient color
    else if (keyword == "Ka") {
        GLfloat red   = parseFloat(line_num, tokens[0], line);
        GLfloat green = parseFloat(line_num, tokens[1], line);
        GLfloat blue  = parseFloat(line_num, tokens[2], line);
        if (pOut_material != nullptr) {
            pOut_material->setAmbient(MyColor(red, green, blue));
        }
        return "";
    }

    // diffuse color
    else if (keyword == "Kd") {
        GLfloat red   = parseFloat(line_num, tokens[0], line);
        GLfloat green = parseFloat(line_num, tokens[1], line);
        GLfloat blue  = parseFloat(line_num, tokens[2], line);
        if (pOut_material != nullptr) {
            pOut_material->setDiffuse(MyColor(red, green, blue));
        }
        return "";
    }

    // emmisive color
    else if (keyword == "Ke") {
        GLfloat red   = parseFloat(line_num, tokens[0], line);
        GLfloat green = parseFloat(line_num, tokens[1], line);
        GLfloat blue  = parseFloat(line_num, tokens[2], line);
        if (pOut_material != nullptr) {
            pOut_material->setEmmissive(MyColor(red, green, blue));
        }
        return "";
    }

    // specular color
    else if (keyword == "Ks") {
        GLfloat red   = parseFloat(line_num, tokens[0], line);
        GLfloat green = parseFloat(line_num, tokens[1], line);
        GLfloat blue  = parseFloat(line_num, tokens[2], line);
        if (pOut_material != nullptr) {
            pOut_material->setSpecular(MyColor(red, green, blue));
        }
        return "";
    }

    // specular exponenent
    else if (keyword == "Ns") {
        GLfloat val = parseFloat(line_num, tokens[0], line);
        if (pOut_material != nullptr) {
            pOut_material->setSpecularExponent(val);
        }
        return "";
    }

    // transparency, one way.
    else if (keyword == "d") {
        GLfloat val = parseFloat(line_num, tokens[0], line);
        if (pOut_material != nullptr) {
            pOut_material->setTransparency(1.0f - val);
        }
        return "";
    }

    // transparency, the other way
    else if (keyword == "Tr") {
        GLfloat val = parseFloat(line_num, tokens[0], line);
        if (pOut_material != nullptr) {
            pOut_material->setTransparency(val);
        }
        return "";
    }

    // refraction index
    else if (keyword == "Ni") {
        GLfloat val = parseFloat(line_num, tokens[0], line);
        if (pOut_material != nullptr) {
            pOut_material->setRefractionIndex(val);
        }
        return "";
    }

    // transmission filter
    else if (keyword == "Tf") {
        GLfloat red   = parseFloat(line_num, tokens[0], line);
        GLfloat green = parseFloat(line_num, tokens[1], line);
        GLfloat blue  = parseFloat(line_num, tokens[2], line);
        if (pOut_material != nullptr) {
            pOut_material->setTransmissionFilter(MyColor(red, green, blue));
        }
        return "";
    }

    // Ignore any other keywords.
    else {
        PrintDebug(fmt::format(
            "Line {0} has a keyword we ignore: {1}'\n",
            line_num, line));
        return "";
    }
}


// Parse a floating point value.
GLfloat ObjFileReader::parseFloat(int line_num, const std::string &val, const std::string &line)
{
    if (boost::regex_match(val, re_float)) {
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
    if (boost::regex_match(val, re_integer)) {
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
    std::string result = fmt::format("OBJ File: {}\n", m_OBJ_fname);

    result += fmt::format(
        "    Dimensions = {0:.3f}m wide by {1:.3f}m deep x {2:.3f}m high\n",
        width, depth, height);

    result += fmt::format("    Vertex count = {}\n", m_vertices.size());
    result += fmt::format("    Normal count = {}\n", m_normals.size());

    return result;
}
