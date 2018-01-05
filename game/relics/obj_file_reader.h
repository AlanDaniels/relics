#pragma once

#include "stdafx.h"

#include "my_math.h"


// A material from a Wavefront ".MTL" file.
// For now, we just want a name and a texture map.
class ObjMaterial
{
public:
    ObjMaterial(const std::string &name) : 
        m_name(name) {}

    ~ObjMaterial() {}

    const std::string &getName()  const { return m_name; }
    const sf::Image   *getImage() const { return m_image.get(); }

    void setImage(std::unique_ptr<sf::Image> image) {
        m_image = std::move(image);
    }

private:
    DISALLOW_DEFAULT(ObjMaterial)
    DISALLOW_COPYING(ObjMaterial)
    DISALLOW_MOVING(ObjMaterial)

    std::string m_name;
    std::unique_ptr<sf::Image> m_image;
};


// The parsed contents of a Wavefront ".OBJ" file.
class ObjFileReader
{
public:
    static std::unique_ptr<ObjFileReader> Create(const std::string &obj_file_name);

    std::string toDescr() const;

private:
    DISALLOW_DEFAULT(ObjFileReader)
    DISALLOW_COPYING(ObjFileReader)
    DISALLOW_MOVING(ObjFileReader)

    // A private ctor, since "Create" does the work.
    ObjFileReader(const std::string &obj_file_name);

    // Private methods.
    bool parseObjLine(int line_num, const std::string &line);

    bool parseMtllibFile(const std::string &partial_MTL_fname);
    std::string parseMtllibLine(int line_num, const std::string &line, ObjMaterial *pOut_material);

    std::string locateRelativeFile(const std::string &relative_fname);
    GLfloat parseFloat(int line_num, const std::string &val, const std::string &line);
    int parseInt(int line_num, const std::string &val, const std::string &line);

    // Private data.
    std::string m_OBJ_file_name;

    std::string m_current_group_name;

    std::vector<MyVec4> m_vertices;
    std::vector<MyVec4> m_normals;
    std::vector<MyVec2> m_tex_coords;

    std::vector<std::unique_ptr<ObjMaterial>> m_materials;
};
