
#include "stdafx.h"
#include "obj_file_reader.h"

#include "common_util.h"
#include "format.h"

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
ObjFileReader::ObjFileReader(const std::string &file_name) :
    m_file_name(file_name),
    m_object_name(""),
    m_current_group_name("")
{
    PrintDebug(fmt::format("Parsing OBJ file '{}'...\n", file_name));
    std::ifstream infile(file_name);

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
    // Skip empty lines and comments.
    if (line.empty() || (line.at(0) == '#')) {
        return;
    }

    // Split the line by spaces.
    boost::sregex_token_iterator iter(line.begin(), line.end(), re_spaces, -1);
    boost::sregex_token_iterator end_thingy;

    // The first token is the keyword.
    std::string keyword = *iter++;

    // The rest of the tokens are data.
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

    // Deal with all the possible keywords.

    // vertex
    if (keyword == "v") {
        GLfloat x = parseFloat(line_num, tokens[0], line);
        GLfloat y = parseFloat(line_num, tokens[1], line);
        GLfloat z = parseFloat(line_num, tokens[2], line);
        m_vertices.emplace_back(x, y, z);
    }

    // vertex normal
    else if (keyword == "vn") {
        GLfloat x = parseFloat(line_num, tokens[0], line);
        GLfloat y = parseFloat(line_num, tokens[1], line);
        GLfloat z = parseFloat(line_num, tokens[2], line);
        m_normals.emplace_back(x, y, z);
    }

    // texture coord
    else if (keyword == "vt") {
        GLfloat u = parseFloat(line_num, tokens[0], line);
        GLfloat v = parseFloat(line_num, tokens[1], line);
        m_tex_coords.emplace_back(u, v);
    }

    // Faces.
    else if (keyword == "f") {
    }

    // object name
    if (keyword == "o") {
        m_object_name = tokens[0];
    }

    // group name
    else if (keyword == "g") { 
        m_current_group_name = tokens[0];
    }

    // smoothing group
    else if (keyword == "s") { 
        const std::string &value = tokens[0];
    }

    // material library
    else if (keyword == "mtllib") {
        parseMtllibFile(tokens[0]);
    }

    // material name
    else if (keyword == "usemtl") {
    }

    // Ignore any other keywords.
    else {
        PrintDebug(fmt::format(
            "Line {0} has a keyword we ignore: {1}'\n",
            line_num, line));
    }
}


void ObjFileReader::parseMtllibFile(const std::string &file_name)
{
    PrintDebug(file_name);
}


void ObjFileReader::parseMtllibLine(int line_num, const std::string &line)
{

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
    std::string result = "";
    result += fmt::format("OBJ File: {}\n", m_file_name);
    result += fmt::format("    Vertex count = {}\n", m_vertices.size());
    result += fmt::format("    Normal count = {}\n", m_normals.size());

    return result;
}
