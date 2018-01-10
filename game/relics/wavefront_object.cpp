
#include "stdafx.h"
#include "wavefront_object.h"

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


// Parsing errors.
static const std::string PARSING_ERROR = "ERROR!";


// Our factory. We hide the constructor in case there's an issue with the file.
std::unique_ptr<WFObject> WFObject::Create(const std::string &resource_path)
{
    // Make the file name absolute.
    std::string full_path = RESOURCE_PATH;
    full_path.append(resource_path);

    // Make sure the file exists.
    boost::filesystem::path our_path(full_path);
    if (!boost::filesystem::is_regular_file(our_path)) {
        PrintDebug(fmt::format("OBJ file {0} does not exist!\n", resource_path));
        assert(false);
        return nullptr;
    }

    std::string final_path = our_path.string();

    // Parse the file. If there are any issues, return null.
    PrintDebug(fmt::format("Parsing OBJ file: {}\n", final_path));

    // TODO: Why can't I use "std::make_unique" here? 
    // The compiler still looks for default ctor.
    std::unique_ptr<WFObject> result(new WFObject(final_path));

    std::ifstream infile(final_path);
    int line_num = 1;
    std::string line;
    while (std::getline(infile, line)) {
        trim_in_place(line);

        if (!result->parseObjectLine(line_num, line)) {
            return nullptr;
        }

        line_num++;
    }

    // Now that we're done parsing, set the object name by hand.
    // Alas, the .OBJ file itself is unreliable for having this.
    result->m_object_name = our_path.filename().stem().string();

    // Otherwise we're okay! Have a great day.
    result->conclude();
    return std::move(result);
}


// The only allowed constructor.
WFObject::WFObject(const std::string &path) :
    m_original_path(path),
    m_object_name(""),
    m_current_group_name("")
{
}


// Now that we've loaded a Wavefront Object from a file, make a copy and 
// move it around as needed. Don't carry along any data you don't need.
// I'm nervous about C++ doing thing behind my back, so I'm doing the copying loops by hand.
std::unique_ptr<WFObject> WFObject::clone(const MyVec4 &move)
{
    std::unique_ptr<WFObject> result(new WFObject(m_original_path));

    result->m_object_name = m_object_name;

    for (const auto &iter : m_mat_map) {
        result->m_mat_map.emplace(iter.first, iter.second);
    }

    for (const auto &iter : m_group_names) {
        result->m_group_names.emplace_back(iter);
    }

    for (const auto &iter : m_group_map) {
        const auto &group_name = iter.first;
        const auto &material   = iter.second->getMaterial();
        const auto &vert_list  = iter.second->getVertList();

        std::unique_ptr<WFGroup> new_group = std::make_unique<WFGroup>(group_name);
        new_group->setMaterial(material);

        for (const auto &vit : vert_list.getVerts()) {
            new_group->add(vit);
        }

        result->m_group_map[group_name] = std::move(new_group);
    }

    result->conclude();
    return std::move(result);
}


// We're rolling our own string space tokenizer, since Boost regex is ridiculously slow.
// We deliberately don't save empty strings, and we're careful about trailing spaces too.
// It appears to work, but feel free to test the living crap out of this later.
std::vector<std::string> WFObject::splitStrBySpaces(const std::string &val)
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
std::vector<std::string> WFObject::splitStrBySlashes(const std::string &val)
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
bool WFObject::parseObjectLine(int line_num, const std::string &line)
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
        std::string name = m_current_group_name;

        if (m_group_map.find(name) == m_group_map.end()) {
            createFaceGroup(line_num, name);
        }

        // Add the faces.
        auto &face_group = m_group_map.at(name);
        face_group->add(parseFaceToken(tokens[1]));
        face_group->add(parseFaceToken(tokens[2]));
        face_group->add(parseFaceToken(tokens[3]));
        return true;
    }

    // Group name.
    // Create a new face group.
    else if (keyword == "g") { 
        m_current_group_name = tokens[1];
        createFaceGroup(line_num, tokens[1]);
        return true;
    }

    // Material library.
    else if (keyword == "mtllib") {
        bool result = parseMtllibFile(tokens[1]);
        return result;
    }

    // Material name.
    // Use this for the current face group. Note that the "current"
    // face group might be the default blank, so create it if needed.
    else if (keyword == "usemtl") {
        const auto &iter = m_mat_map.find(tokens[1]);
        if (iter == m_mat_map.end()) {
            PrintDebug(fmt::format(
                "Line {0}: Referenced material {1} is never defined!\n",
                line_num, tokens[1]));
        }

        std::shared_ptr<WFMaterial> mat = iter->second;

        if (m_group_map.find(m_current_group_name) == m_group_map.end()) {
            createFaceGroup(line_num, m_current_group_name);
        }

        m_group_map.at(m_current_group_name)->setMaterial(mat);
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
        (keyword == "o") ||          // Object name (just roll our own)
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


// Create a new face group.
void WFObject::createFaceGroup(int line_num, const std::string &name)
{
    const auto &iter = m_group_map.find(name);
    if (iter != m_group_map.end()) {
        PrintDebug(fmt::format(
            "Line {0}: Tried to add group '{1}' twice. Ignoring.\n",
            line_num, name));
        return;
    }

    m_group_names.emplace_back(name);
    m_group_map.emplace(name, std::make_unique<WFGroup>(name));
}


// Parse a token from a "face" line. We break this out  since the logic is 
// complicated. Account for blank values, and the "offset by 1" business.
// Side note: It's annoying that std::vector returns an "unsigned int" for size.
// If your vector ever really does grow beyond 2GB entries, you have bigger problems.
Vertex_PNT WFObject::parseFaceToken(const std::string &token)
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
bool WFObject::parseMtllibFile(const std::string &path)
{
    std::string real_path = locateRelativeFile(path);
    if (real_path == "") {
        return false;
    }

    // We build material objects as we go.
    std::unique_ptr<WFMaterial> material = nullptr;

    // Parse the file line by line.
    PrintDebug(fmt::format("Parsing MTL file: {}\n", real_path));
    std::ifstream infile(real_path);

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
                m_mat_map.emplace(key, std::move(material));
            }

            material = std::make_unique<WFMaterial>(new_name);
        }

        line_num++;
    }

    // All done. Save any final material.
    if (material != nullptr) {
        const std::string key = material->getMaterialName();
        m_mat_map.emplace(key, std::move(material));
    }

    return true;
}


// Parse a line in our MTL file.
// Returning a string signals the beginning of a new material.
// Returning a blank means we're still in the current material.
// Ugly, but returning the word "ERROR" means there's an issue with the MTL file.
std::string WFObject::parseMtllibLine(int line_num, const std::string &line, WFMaterial *pOut_material)
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
            std::string image_path = locateRelativeFile(tokens[1]);
            if (image_path == "") {
                return PARSING_ERROR;
            }

            std::unique_ptr<DrawTexture> draw_texture = 
                std::make_unique<DrawTexture>(image_path);

            pOut_material->setDrawTexture(std::move(draw_texture));
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
std::string WFObject::locateRelativeFile(const std::string &rel_path)
{
    boost::filesystem::path starter = boost::filesystem::canonical(m_original_path);
    boost::filesystem::path wanted  = starter.parent_path().append(rel_path);

    if (!boost::filesystem::is_regular_file(wanted)) {
        PrintDebug(fmt::format("Referenced file {0} does not exist!\n", rel_path));
        return "";
    }

    return wanted.string();
}


// Return a description string.
std::string WFObject::toDescr() const
{
    // List the Object file details.
    std::string result = fmt::format("OBJ File: '{}'\n", m_object_name);
    result += fmt::format("    File path = {}\n", m_original_path);

    // Figure out our object dimensions.
    if (m_positions.size() > 0) {
        GLfloat min_x = FLT_MAX;
        GLfloat min_y = FLT_MAX;
        GLfloat min_z = FLT_MAX;

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

        result += fmt::format(
            "    Dimensions = {0:.3f}m wide, {1:.3f}m deep, {2:.3f}m high\n",
            width, depth, height);
        result += fmt::format("    Position count = {}\n", m_positions.size());
    }

    if (m_normals.size() > 0) {
        result += fmt::format("    Normal count = {}\n", m_normals.size());
    }

    if (m_tex_coords.size() > 0) {
        result += fmt::format("    Tex Coord count = {}\n", m_tex_coords.size());
    }

    // List the materials.
    if (m_mat_map.size() > 0) {
        result += "    Materials:\n";
        for (const auto &iter : m_mat_map) {
            result += fmt::format(
                "        Name = '{0}', Texture = '{1}'\n",
                iter.first, iter.second->getDrawTexture()->getFileName());
        }
    }
    else {
        result += "    ERROR! No materials.\n";
    }

    // List the face groups.
    if (m_group_map.size() > 0) {
        int total_faces = 0;
        result += "    Face Groups:\n";
        for (const auto &iter : m_group_map) {
            std::string group_name = iter.first;
            assert(iter.second->getMaterial() != nullptr);
            const auto &mat_name   = iter.second->getMaterial()->getMaterialName();
            const auto &vert_list  = iter.second->getVertList();

            if (group_name == "") {
                group_name = "[BLANK]";
            }

            result += fmt::format(
                "        '{0}' = Using '{1}', with {2} face vertices\n",
                group_name, mat_name, vert_list.getItemCount());
            total_faces += vert_list.getItemCount();
        }

        result += fmt::format(
            "    Total vertex count = {0} ({1} triangles)\n", 
            total_faces, total_faces / 3);
        if ((total_faces % 3) != 0) {
            result += "    ERROR! Number of Face Vertices is NOT divisible by 3!\n";
        }
    }
    else {
        result += "    ERROR! No faces! WTF?!\n";
    }

    return result;
}


// Now that we've loaded our Wavefront object, validate
// that everything is okay and build our underlying Draw State stuff.
bool WFObject::conclude() const
{
    bool success = true;

    // Check the object name.
    if (m_object_name == "") {
        PrintDebug("ERROR: Object name wasn't set!\n");
        success = false;
    }

    // Check each material.
    if (m_mat_map.size() == 0) {
        PrintDebug("ERROR: No materials!\n");
        success = false;
    }

    for (const auto &iter : m_mat_map) {
        const auto &mat = iter.second;
        if ((mat->getMaterialName() == "") ||
            (mat->getDrawTexture()  == nullptr)) {
            PrintDebug("ERROR: Bad material!\n");
            success = false;
        }
    }

    // Face Group names.
    if (m_group_names.size() == 0) {
        PrintDebug("ERROR: No group names!\n");
        success = false;
    }

    // Face Group map.
    for (const auto &iter : m_group_map) {
        const auto &group = iter.second;
        if (group->getMaterial() == nullptr) {
            PrintDebug("ERROR: Blank material!\n");
            success = false;
        }

        if (group->getVertList().getItemCount() == 0) {
            PrintDebug("ERROR: No vertices!\n");
            success = false;
        }
    }

    // All done.
    if (!success) {
        assert(false);
        return false;
    }

    // Update each Face Group vertex list.
    for (auto &iter : m_group_map) {
        iter.second->updateVertList();
    }
    return success;
}