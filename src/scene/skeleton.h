
#pragma once

#include <functional>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../geometry/spline.h"
#include "../lib/mathlib.h"
#include "../platform/gl.h"

class Joint {
public:
    Joint(unsigned int id) : _id(id) {}
    Joint(unsigned int id, Joint *parent, Vec3 extent) : _id(id), parent(parent), extent(extent) {}
    ~Joint() {
        for (Joint *j : children)
            delete j;
    }

    Joint(const Joint &src) = delete;
    Joint(Joint &&src) = default;

    void operator=(const Joint &src) = delete;
    Joint &operator=(Joint &&src) = default;

    unsigned int id() const { return _id; }

    // Checks if this joint is a root node
    bool is_root() const;

    // Euler angles representing the current joint rotation
    Vec3 pose;

    // The vector representing the direction and length of the bone.
    // This is specified in Joint space, and defines the origin of child bones.
    Vec3 extent = Vec3(0.0f, 1.0f, 0.0f);

    // The distance at which the joint segment should stop effecting vertices
    float radius = 0.25f;

private:
    // Builds the transformation matrix that takes a point in joint space to the mesh
    // space (in bind position). "Bind" position implies that the rotation of all joints
    // should be zero. Also note that this does not include the Skeleton's base_pos,
    // but it should include the transformations of the joint heirachy up to this point.
    Mat4 joint_to_bind() const;

    // Similarly, builds the transformation that takes a point in the space of this joint
    // into mesh space - taking into account the poses of the joint heirarchy. This also does
    // not include the Skeleton's base_pos.
    Mat4 joint_to_posed() const;

    // Pointer to parent joint in the joint heirarchy
    Joint *parent = nullptr;

    // Set of child joints - owned by this joint (could be shared_ptr and everything else weak_ptr)
    std::unordered_set<Joint *> children;

    void for_joints(std::function<void(Joint *)> func);

    unsigned int _id = 0;
    Spline<Quat> anim;

    friend class Skeleton;
    friend class Scene;
};

class Skeleton {
public:
    Skeleton();
    Skeleton(unsigned int obj_id);
    ~Skeleton();

    Skeleton(const Skeleton &src) = delete;
    Skeleton(Skeleton &&src) = default;

    void operator=(const Skeleton &src) = delete;
    Skeleton &operator=(Skeleton &&src) = default;

    Vec3 &base();
    bool has_bones() const;
    unsigned int n_bones();

    Joint *parent(Joint *j);
    Joint *get_joint(unsigned int id);
    void erase(Joint *j);
    void restore(Joint *j);
    Vec3 end_of(Joint *j);
    Vec3 base_of(Joint *j);
    Vec3 posed_end_of(Joint *j);
    Vec3 posed_base_of(Joint *j);

    void for_joints(std::function<void(Joint *)> func);

    Mat4 joint_to_bind(const Joint *j) const;
    Mat4 joint_to_posed(const Joint *j) const;

    Joint *add_root(Vec3 extent);
    Joint *add_child(Joint *j, Vec3 extent);
    bool is_root_id(unsigned int id);

    bool set_time(float time);
    void render(const Mat4 &view, Joint *select, bool root, bool posed, unsigned int offset = 0);
    void outline(const Mat4 &view, Joint *select, bool root, bool posed, BBox &box,
                 unsigned int offset = 0);
    void find_joints(const GL::Mesh &src,
                     std::unordered_map<unsigned int, std::vector<Joint *>> &map);
    void skin(const GL::Mesh &input, GL::Mesh &output,
              const std::unordered_map<unsigned int, std::vector<Joint *>> &map);

    void set(float t);
    void crop(float t);
    void erase(float t);
    bool has_keyframes();
    std::set<float> keys();
    std::unordered_map<unsigned int, Vec3> now();
    std::unordered_map<unsigned int, Vec3> at(float t);
    void set(float t, const std::unordered_map<unsigned int, Vec3> &data);

private:
    Vec3 base_pos;
    unsigned int root_id, next_id;
    std::unordered_set<Joint *> roots, erased;
    friend class Scene;
};
