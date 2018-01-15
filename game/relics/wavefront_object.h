#pragma once

#include "stdafx.h"

#include "my_math.h"
#include "draw_state_pnt.h"
#include "draw_texture.h"


// A material from a Wavefront ".MTL" file.
// For now, we just want a name and a texture map.
class WFMaterial
{
public:
    WFMaterial(const std::string &name) :
        m_mat_name(name),
        m_draw_texture(nullptr) {}

    ~WFMaterial() {}

    const std::string &getMaterialName() const { return m_mat_name; }
    const DrawTexture *getDrawTexture()  const { return m_draw_texture.get(); }

    void setDrawTexture(std::unique_ptr<DrawTexture> draw_texture) {
        m_draw_texture = std::move(draw_texture);
    }

private:
    FORBID_DEFAULT_CTOR(WFMaterial)
    FORBID_COPYING(WFMaterial)
    FORBID_MOVING(WFMaterial)

    // Private data.
    std::string m_mat_name;
    std::unique_ptr<DrawTexture> m_draw_texture;
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
        m_mat (std::move(that.m_mat)),
        m_vert_list(std::move(that.m_vert_list)) {}

    ~WFGroup() {}

    WFGroup &operator=(WFGroup &&that) {
        m_name = std::move(m_name);
        m_mat  = std::move(m_mat);
        m_vert_list = std::move(m_vert_list);
        return *this;
    }

    const std::shared_ptr<WFMaterial> &getMaterial() const { 
        return m_mat; 
    }

    const VertList_PNT &getVertList() const { 
        return m_vert_list; 
    }

    void setMaterial(std::shared_ptr<WFMaterial> mat) {
        m_mat = mat;
    }

    void add(const Vertex_PNT &vert) {
        m_vert_list.add(&vert, 1);
    }

    void updateVertList() {
        m_vert_list.update();
    }

private:
    FORBID_COPYING(WFGroup)

    // Privata data. Note that materials are shared, but vert lists aren't.
    std::string m_name;
    std::shared_ptr<WFMaterial> m_mat;
    VertList_PNT m_vert_list;
};


// The parsed contents of a Wavefront ".OBJ" file.
// Side note: I tried using Boost regex to deal with splitting file lines
// by spaces, and parsing floats and ints, but it was unacceptably slow.
class WFInstance;

class WFObject
{
public:
    static std::unique_ptr<WFObject> Create(const std::string &resource_path);

    std::unique_ptr<WFInstance> clone(const MyVec4 &move);

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
        return *m_group_map.at(name);
    }

    std::string toDescr() const;
    bool conclude() const;

private:
    FORBID_DEFAULT_CTOR(WFObject)
    FORBID_COPYING(WFObject)
    FORBID_MOVING(WFObject)

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

    std::map<std::string, std::shared_ptr<WFMaterial>> m_mat_map;

    std::vector<std::string> m_group_names;
    std::map<std::string, std::unique_ptr<WFGroup>> m_group_map;
};


// An instance of a Wavefront object.
// TODO: Down the road, this will need its own geometry for
// collision detection, but worry about that then.
class WFInstance
{
public:
    WFInstance(const WFObject &original, const MyVec4 &move) :
        m_original(original), m_move(move) {}

    ~WFInstance() {}

    DEFAULT_MOVING(WFInstance)

    const WFObject &getOriginal() const { return m_original; }
    const MyVec4 &getMove() const { return m_move; }

private:
    FORBID_DEFAULT_CTOR(WFInstance)
    FORBID_COPYING(WFInstance)

    // Private data.
    const WFObject &m_original;
    MyVec4 m_move;
};