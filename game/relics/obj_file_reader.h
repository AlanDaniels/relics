#pragma once

#include "stdafx.h"

#include "my_math.h"


// TODO: We'll probably need a Vec4 here that has a name and index.


// An MTL file material. We probably won't need every field mentioned
// in the file spec, but for now, just include everything.
class ObjMaterial
{
public:
    ObjMaterial(const std::string &name) : 
        m_name(name),
        m_ambient(1, 1, 1),
        m_diffuse(1, 1, 1),
        m_emmissive(0, 0, 0),
        m_specular(0, 0, 0),
        m_specular_exponent(10.0f),
        m_transparency(0),
        m_refraction_index(1),
        m_transmission_filter(1, 1, 1) {}

    ~ObjMaterial() {}

    void setAmbient(const MyColor &val)   { m_ambient   = val; }
    void setDiffuse(const MyColor &val)   { m_diffuse   = val; }
    void setEmmissive(const MyColor &val) { m_emmissive = val; }
    void setSpecular(const MyColor &val)  { m_specular  = val; }
    void setSpecularExponent(GLfloat val) { m_specular_exponent = val; }
    void setTransparency(GLfloat val)     { m_transparency = val; }
    void setRefractionIndex(GLfloat val)  { m_refraction_index = val; }
    void setTransmissionFilter(const MyColor &val) { m_transmission_filter = val; }

    const std::string &getName()  const { return m_name; }
    const MyColor &getAmbient()   const { return m_ambient; }
    const MyColor &getDiffuse()   const { return m_diffuse; }
    const MyColor &getEmmissive() const { return m_emmissive; }
    const MyColor &getSpecular()  const { return m_specular; }
    GLfloat getSpecularExponent() const { return m_specular_exponent; }
    GLfloat getTransparency()     const { return m_transparency; }
    GLfloat getRefractionIndex()  const { return m_refraction_index; }
    const MyColor &getTransmissionFilter() const { return m_transmission_filter; }

private:
    DISALLOW_DEFAULT(ObjMaterial)
    DISALLOW_COPYING(ObjMaterial)
    DISALLOW_MOVING(ObjMaterial)

    std::string m_name;
    MyColor m_ambient;
    MyColor m_diffuse;
    MyColor m_emmissive;
    MyColor m_specular;
    GLfloat m_specular_exponent;
    GLfloat m_transparency;
    GLfloat m_refraction_index;
    MyColor m_transmission_filter;
};


class ObjFileReader
{
public:
    ObjFileReader(const std::string &partial_OBJ_fname);

    std::string toDescr() const;

private:
    DISALLOW_DEFAULT(ObjFileReader)
    DISALLOW_COPYING(ObjFileReader)
    DISALLOW_MOVING(ObjFileReader)

    // Private methods.
    void parseObjLine(int line_num, const std::string &line);

    void parseMtllibFile(const std::string &partial_MTL_fname);
    std::string parseMtllibLine(int line_num, const std::string &line, ObjMaterial *pOut_material);

    GLfloat parseFloat(int line_num, const std::string &val, const std::string &line);
    int parseInt(int line_num, const std::string &val, const std::string &line);

    // Private data.
    std::string m_OBJ_fname;
    std::string m_object_name;

    std::string m_current_group_name;

    std::vector<MyVec4> m_vertices;
    std::vector<MyVec4> m_normals;
    std::vector<MyVec2> m_tex_coords;

    std::vector<std::unique_ptr<ObjMaterial>> m_materials;
};
