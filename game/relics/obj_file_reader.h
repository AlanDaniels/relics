#pragma once

#include "stdafx.h"

#include "my_math.h"
#include "draw_state_pnt.h"


// A material from a Wavefront ".MTL" file.
// For now, we just want a name and a texture map.
class ObjMaterial
{
public:
    ObjMaterial(const std::string &name) : 
        m_material_name(name),
        m_texture_file_name(""),
        m_texture_image(nullptr) {}

    ~ObjMaterial() {}

    const std::string &getMaterialName() const { return m_material_name; }
    const std::string &getTextureFileName() const { return m_texture_file_name; }
    const sf::Image   *getTextureImage() const { return m_texture_image.get(); }

    void setTexture(const std::string &file_name, std::unique_ptr<sf::Image> image) {
        m_texture_file_name = file_name;
        m_texture_image = std::move(image);
    }

private:
    DISALLOW_DEFAULT(ObjMaterial)
    DISALLOW_COPYING(ObjMaterial)
    DISALLOW_MOVING(ObjMaterial)

    std::string m_material_name;
    std::string m_texture_file_name;
    std::unique_ptr<sf::Image> m_texture_image;
};


class ObjFaceGroup
{
public:
    ObjFaceGroup() :
        m_group_name(""),
        m_material_name("") {}

    ObjFaceGroup(const std::string &group_name, 
                 const std::string &material_name) :
        m_group_name   (group_name),
        m_material_name(material_name) {}

    ObjFaceGroup(const ObjFaceGroup &that) :
        m_group_name   (that.m_group_name),
        m_material_name(that.m_material_name) {}

    ObjFaceGroup(ObjFaceGroup &&that) :
        m_group_name   (std::move(that.m_group_name)),
        m_material_name(std::move(that.m_material_name)) {}

    ObjFaceGroup &operator=(const ObjFaceGroup &that) {
        m_group_name    = that.m_group_name;
        m_material_name = that.m_material_name;
        return *this;
    }

    ObjFaceGroup &operator=(ObjFaceGroup &&that) {
        m_group_name    = std::move(that.m_group_name);
        m_material_name = std::move(that.m_material_name);
        return *this;
    }

    const std::string &getGroupName()    const { return m_group_name; }
    const std::string &getMaterialName() const { return m_material_name; }

    ~ObjFaceGroup() {}

private:
    std::string m_group_name;
    std::string m_material_name;
};


// Less-than operator, since we'll be using Face Groups as a map key.
bool operator<(const ObjFaceGroup &one, const ObjFaceGroup &two);



// The parsed contents of a Wavefront ".OBJ" file.
// Side note: I tried using Boost regex to deal with splitting file lines
// by spaces, and parsing floats and ints, but it was unacceptably slow.
class ObjFileReader
{
public:
    static std::unique_ptr<ObjFileReader> Create(const std::string &obj_file_name);

    const std::string &getFileName() const { return m_file_name; }
    const std::string &getObjectName() const { return m_object_name; }

    std::string toDescr() const;

private:
    DISALLOW_DEFAULT(ObjFileReader)
    DISALLOW_COPYING(ObjFileReader)
    DISALLOW_MOVING(ObjFileReader)

    // A private ctor, since "Create" does the work.
    ObjFileReader(const std::string &obj_file_name);

    // Private methods.
    std::vector<std::string> splitStrBySpaces(const std::string &val);
    std::vector<std::string> splitStrBySlashes(const std::string &val);

    bool parseObjLine(int line_num, const std::string &line);
    
    Vertex_PNT parseFaceToken(const std::string &token);

    bool parseMtllibFile(const std::string &partial_MTL_fname);
    std::string parseMtllibLine(int line_num, const std::string &line, ObjMaterial *pOut_material);

    std::string locateRelativeFile(const std::string &relative_fname);

    GLfloat parseFloat(const std::string &val) const {
        return static_cast<GLfloat>(::atof(val.c_str()));
    }

    // Private data.
    std::string m_file_name;
    std::string m_object_name;

    std::string m_current_group_name;
    std::string m_current_material_name;

    std::vector<MyVec4> m_positions;
    std::vector<MyVec4> m_normals;
    std::vector<MyVec2> m_tex_coords;

    std::map<std::string, std::unique_ptr<ObjMaterial>> m_materials_map;

    std::map<ObjFaceGroup, std::unique_ptr<std::vector<Vertex_PNT>>> m_face_groups_map;
};
