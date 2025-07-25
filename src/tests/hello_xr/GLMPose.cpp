//#include "pch.h"

#include "GLMPose.h"
#include <cstring>

#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/intersect.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

namespace BVR
{
XrMatrix4x4f convert_to_xr(const glm::mat4& input)
{
    XrMatrix4x4f output;
    memcpy(&output, &input, sizeof(output));
    return output;
}

glm::mat4 convert_to_glm(const XrMatrix4x4f& input)
{
    glm::mat4 output;
    memcpy(&output, &input, sizeof(output));
    return output;
}

XrVector3f convert_to_xr(const glm::vec3 &input)
{
    XrVector3f output;
    std::memcpy(&output, &input, sizeof(output));
    return output;
}

glm::vec3 convert_to_glm(const XrVector3f &input)
{
    glm::vec3 output;
    output.x = input.x;
    output.y = input.y;
    output.z = input.z;
    return output;
}

XrQuaternionf convert_to_xr(const glm::fquat &input)
{
    XrQuaternionf output;
    output.x = input.x;
    output.y = input.y;
    output.z = input.z;
    output.w = input.w;
    return output;
}

glm::fquat convert_to_glm(const XrQuaternionf &input)
{
    glm::fquat output;
    output.x = input.x;
    output.y = input.y;
    output.z = input.z;
    output.w = input.w;
    return output;
}


GLMPose convert_to_glm(const XrVector3f& position, const XrQuaternionf& rotation, const XrVector3f& scale)
{
	GLMPose glm_pose;
	glm_pose.translation_ = convert_to_glm(position);
	glm_pose.rotation_ = convert_to_glm(rotation);
	glm_pose.scale_ = convert_to_glm(scale);
	return glm_pose;
}

GLMPose convert_to_glm(const XrPosef& xr_pose)
{
	GLMPose glm_pose;
	glm_pose.translation_ = convert_to_glm(xr_pose.position);
	glm_pose.rotation_ = convert_to_glm(xr_pose.orientation);
	return glm_pose;
}

XrPosef convert_to_xr(const GLMPose& glm_pose)
{
	// No scale
	XrPosef xr_pose;
	xr_pose.position = convert_to_xr(glm_pose.translation_);
	xr_pose.orientation = convert_to_xr(glm_pose.rotation_);
	return xr_pose;
}


glm::mat4 convert_to_rotation_matrix(const glm::fquat &rotation)
{
    glm::mat4 rotation_matrix = glm::mat4_cast(rotation);
    return rotation_matrix;
}

void GLMPose::clear()
{
    translation_ = glm::vec3(0.0f, 0.0f, 0.0f);
    rotation_ = default_rotation;
    scale_ = glm::vec3(1.0f, 1.0f, 1.0f);
    euler_angles_degrees_ = glm::vec3(0.0f, 0.0f, 0.0f);
}

glm::mat4 GLMPose::to_matrix() const
{
    glm::mat4 translation_matrix = glm::translate(glm::mat4(1), translation_);
    glm::mat4 rotation_matrix = glm::mat4_cast(rotation_);
    glm::mat4 scale_matrix = glm::scale(scale_);
    return translation_matrix * rotation_matrix * scale_matrix;
}

void GLMPose::update_rotation_from_euler()
{
    glm::vec3 euler_angles_radians(deg2rad(euler_angles_degrees_.x),
                                   deg2rad(euler_angles_degrees_.y),
                                   deg2rad(euler_angles_degrees_.z));
    rotation_ = glm::fquat(euler_angles_radians);
}


void GLMPose::transform(const GLMPose &glm_pose)
{
    glm::vec3 rotated_translation = rotation_ * glm_pose.translation_;
    translation_ += rotated_translation;
    rotation_ = glm::normalize(rotation_ * glm_pose.rotation_);
}

void GLMPose::project_to_horizontal_plane()
{
    glm::vec3 pose_direction = glm::rotate(rotation_, forward_direction);
    pose_direction.y = 0.0f;
    pose_direction = glm::normalize(pose_direction);

    const glm::fquat rotation_world_2D = glm::rotation(forward_direction, pose_direction);
    rotation_ = rotation_world_2D;
}

GLMPose convert_to_glm_pose(const XrPosef &xr_pose)
{
    GLMPose glm_pose;
    glm_pose.translation_ = convert_to_glm(xr_pose.position);
    glm_pose.rotation_ = convert_to_glm(xr_pose.orientation);
    return glm_pose;
}

XrPosef convert_to_xr_pose(const GLMPose &glm_pose)
{
    // No scale
    XrPosef xr_pose;
    xr_pose.position = convert_to_xr(glm_pose.translation_);
    xr_pose.orientation = convert_to_xr(glm_pose.rotation_);
    return xr_pose;
}

Normal TexturedTriangle::compute_normal() const
{
    return glm::normalize(glm::cross((B_.position_ - A_.position_), (C_.position_ - B_.position_)));
}

TexturedTriangle TexturedTriangle::transform(const glm::mat4& matrix) const
{
    TexturedTriangle transformed_triangle;

    transformed_triangle.A_.position_ = matrix * glm::vec4(A_.position_, 1.0f);
    transformed_triangle.B_.position_ = matrix * glm::vec4(B_.position_, 1.0f);
    transformed_triangle.C_.position_ = matrix * glm::vec4(C_.position_, 1.0f);

    transformed_triangle.A_.texcoord_ = A_.texcoord_;
    transformed_triangle.B_.texcoord_ = B_.texcoord_;
    transformed_triangle.C_.texcoord_ = C_.texcoord_;

    return transformed_triangle;
}

IntersectionResult TexturedTriangle::intersect(const Ray& ray) const
{
    IntersectionResult result;

    const bool was_hit = glm::intersectRayTriangle(ray.origin_, ray.direction_, A_.position_, B_.position_, C_.position_, result.hit_.barycentric_, result.hit_.distance_);
    const bool facing_right_direction = (result.hit_.distance_ > 0.0f);

    if (was_hit && facing_right_direction)
    {
        result.was_hit_ = true;

        result.hit_.position_ = ray.origin_ + (ray.direction_ * result.hit_.distance_);
        result.hit_.normal_ = compute_normal();

        glm::vec3 bary3 = glm::vec3((1.0f - result.hit_.barycentric_.x - result.hit_.barycentric_.y),
                                    result.hit_.barycentric_.x, result.hit_.barycentric_.y);

        result.hit_.uv_ = bary3.x * A_.texcoord_ + bary3.y * B_.texcoord_ + bary3.z * C_.texcoord_;
    }

    return result;
}

Normal Quad::compute_normal() const
{
    return A_.compute_normal();
}

Quad Quad::transform(const glm::mat4& matrix) const
{
    Quad transformed_quad;

    transformed_quad.A_ = A_.transform(matrix);
    transformed_quad.B_ = B_.transform(matrix);

    return transformed_quad;
}

IntersectionResult Quad::intersect(const Ray& ray) const
{
    {
        IntersectionResult result_A = A_.intersect(ray);

        if (result_A.was_hit_)
        {
            return result_A;
        }
    }

    IntersectionResult result_B = B_.intersect(ray);

    if (result_B.was_hit_)
    {
    }

    return result_B;
}


} // namespace BVR



