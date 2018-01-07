#pragma once

#include "stdafx.h"

#include "my_math.h"
#include "draw_state_pnt.h"


// A material from a Wavefront ".MTL" file.
// For now, we just want a name and a texture map.
class WFMaterial
{
public:
    WFMaterial(const std::string &name) :
        m_mat_name(name),
        m_tex_path(""),
        m_tex_image(nullptr) {}

    ~WFMaterial() {
        // TODO: Free up the sf::Image resource.
    }

    const std::string &getMaterialName() const { return m_mat_name; }
    const std::string &getTexturePath()  const { return m_tex_path; }
    const sf::Image   *getTextureImage() const { return m_tex_image.get(); }

    void setTexture(const std::string &path, std::unique_ptr<sf::Image> image) {
        m_tex_path  = path;
        m_tex_image = std::move(image);
    }

private:
    DISALLOW_DEFAULT(WFMaterial)
    DISALLOW_COPYING(WFMaterial)
    DISALLOW_MOVING(WFMaterial)

    // Private data.
    std::string m_mat_name;
    std::string m_tex_path;
    std::unique_ptr<sf::Image> m_tex_image;
};


// A wavefront face group.
// One name maps to a material, and the vertices to go with it.
class WFGroup
{
public:
    WFGroup() {}

    WFGroup(const std::string &name) :
        m_name(name) {}

    WFGroup(WFGroup &&that) :
        m_name(std::move(that.m_name)),
        m_mat(std::move(that.m_mat)),
        m_vert_list(std::move(that.m_vert_list)) {}

    ~WFGroup() {}

    WFGroup &operator=(WFGroup &&that) {
        m_name = std::move(m_name);
        m_mat  = std::move(m_mat);
        m_vert_list = std::move(m_vert_list);
        return *this;
    }

    const std::shared_ptr<WFMaterial> &getMaterial() const { return m_mat; }
    const std::vector<Vertex_PNT>     &getVertList() const { return m_vert_list; }

    void setMaterial(std::shared_ptr<WFMaterial> mat) {
        m_mat = mat;
    }

    void addVertex(const Vertex_PNT &vert) {
        m_vert_list.emplace_back(vert);
    }

private:
    DISALLOW_COPYING(WFGroup)

    // Privata data. Note that materials are shared, but vert lists aren't.
    std::string m_name;
    std::shared_ptr<WFMaterial> m_mat;
    std::vector<Vertex_PNT> m_vert_list;
};


// The parsed contents of a Wavefront ".OBJ" file.
// Side note: I tried using Boost regex to deal with splitting file lines
// by spaces, and parsing floats and ints, but it was unacceptably slow.
class WFObject
{
public:
    static std::unique_ptr<WFObject> Create(const std::string &resource_path);

    std::unique_ptr<WFObject> clone(const MyVec4 &translate);

    const std::string &getPath() const { 
        return m_original_path; 
    }

    const std::string &getObjectName() const { 
        return m_object_name; 
    }

    const std::vector<std::string> &getGroupNames() const {
        return m_group_names;
    }

    const WFGroup &getGroup(const std::string &name) const {
        const auto &iter = m_group_map.at(name);
        return *iter;
    }

    std::string toDescr() const;
    bool validate() const;

private:
    DISALLOW_DEFAULT(WFObject)
    DISALLOW_COPYING(WFObject)
    DISALLOW_MOVING(WFObject)

    // A private ctor, since "Create" does the work.
    WFObject(const std::string &path);

    // Private methods.
    std::vector<std::string> splitStrBySpaces(const std::string &val);
    std::vector<std::string> splitStrBySlashes(const std::string &val);

    bool parseObjectLine(int line_num, const std::string &line);
    void createFaceGroup(int line_num, const std::string &name);
    
    Vertex_PNT parseFaceToken(const std::string &token);

    bool parseMtllibFile(const std::string &partial_MTL_fname);
    std::string parseMtllibLine(int line_num, const std::string &line, WFMaterial *pOut_material);

    std::string locateRelativeFile(const std::string &relative_fname);

    GLfloat parseFloat(const std::string &val) const {
        return static_cast<GLfloat>(::atof(val.c_str()));
    }

    // Private data.
    std::string m_original_path;
    std::string m_object_name;

    std::string m_current_group_name;

    std::vector<MyVec4> m_positions;
    std::vector<MyVec4> m_normals;
    std::vector<MyVec2> m_tex_coords;

    // Keep our list of materials.
    std::map<std::string, std::shared_ptr<WFMaterial>> m_mat_map;

    // Each group carries along its one material.
    std::vector<std::string> m_group_names;
    std::map<std::string, std::unique_ptr<WFGroup>> m_group_map;
};
