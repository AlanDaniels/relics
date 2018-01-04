#pragma once

#include "stdafx.h"

#include "my_math.h"


// TODO: We'll probably need a Vec4 here that has a name and index.


class ObjFileReader
{
public:
    ObjFileReader(const std::string &file_name);

    std::string toDescr() const;

private:
    DISALLOW_DEFAULT(ObjFileReader)
    DISALLOW_COPYING(ObjFileReader)
    DISALLOW_MOVING(ObjFileReader)

    // Private methods.
    void parseObjLine(int line_num, const std::string &line);

    void parseMtllibFile(const std::string &file_name);
    void parseMtllibLine(int line_num, const std::string &line);

    GLfloat parseFloat(int line_num, const std::string &val, const std::string &line);
    int     parseInt(int line_num, const std::string &val, const std::string &line);

    // Private data.
    std::string m_file_name;
    std::string m_object_name;

    std::string m_current_group_name;

    std::vector<MyVec4> m_vertices;
    std::vector<MyVec4> m_normals;
    std::vector<MyVec2> m_tex_coords;
};
