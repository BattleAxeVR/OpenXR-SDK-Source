#ifndef GLM_POSE_H
#define GLM_POSE_H

#include <openxr/openxr.h>
#include "xr_linear.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include <glm/common.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

const float PI = 3.1415927f;
const float TWO_PI = (float)6.28318530717958648;
const float PI_OVER_TWO = (float)1.57079632679489661923;
const float PI_OVER_FOUR = (float)0.785398163397448309616;
const float INV_PI = (float)0.318309886183790671538;
const float TWO_OVER_PI = (float)0.636619772367581343076;

const float EPSILON = 0.00001f;

const float DEG_TO_RAD = PI / 180.0f;
const float RAD_TO_DEG = 180.0f / PI;
const float ROOT_OF_HALF = 0.7071067690849304f;

#ifndef deg2rad
#define deg2rad(a) ((a)*DEG_TO_RAD)
#endif

#ifndef rad2deg
#define rad2deg(a) ((a)*RAD_TO_DEG)
#endif

namespace BVR
{
template<typename T> static inline T clamp(T v, T mn, T mx)
{
    return (v < mn) ? mn : (v > mx) ? mx : v;
}

inline float sign(float val)
{
    return (val < 0.0f) ? -1.0f : 1.0f;
}

const float ROOT_OF_HALF = 0.7071067690849304f;

const glm::fquat default_rotation(1.0f, 0.0f, 0.0f, 0.0f);
const glm::fquat rotate_90_CCW_by_x(0.7071067690849304f, 0.7071067690849304f, 0.0f, 0.0f);
const glm::fquat rotate_180_CCW_about_y(0.0f, 1.0f, 0.0f, 0.0f);
const glm::fquat rotate_CW_45_rotation_about_x(0.9238795f, -0.3826834f, 0.0f, 0.0f);

const glm::fquat CCW_180_rotation_about_y = glm::fquat(0, 1, 0, 0);
const glm::fquat CCW_180_rotation_about_x = glm::fquat(0, 0, 1, 0);
const glm::fquat CCW_180_rotation_about_z = glm::fquat(0, 0, 0, 1);

const glm::fquat CCW_45_rotation_about_y = glm::fquat(0, 0.3826834f, 0, 0.9238795f);
const glm::fquat CW_45_rotation_about_y = glm::fquat(0, -0.3826834f, 0, 0.9238795f);

const glm::fquat CCW_90_rotation_about_y = glm::fquat(0, ROOT_OF_HALF, 0, ROOT_OF_HALF);
const glm::fquat CW_90_rotation_about_y = glm::fquat(0, -ROOT_OF_HALF, 0, ROOT_OF_HALF);

const glm::fquat CW_90_rotation_about_x = glm::fquat(-ROOT_OF_HALF, 0, 0, ROOT_OF_HALF);
const glm::fquat CCW_90_rotation_about_x = glm::fquat(ROOT_OF_HALF, 0, 0, ROOT_OF_HALF);

const glm::fquat CW_30deg_rotation_about_x = glm::fquat(-0.258819f, 0, 0, 0.9659258f);
const glm::fquat CCW_30deg_rotation_about_x = glm::fquat(0.258819f, 0, 0, 0.9659258f);

const glm::fquat CW_30deg_rotation_about_Y = glm::fquat(0.0f, 0.258819f, 0.0f, 0.9659258f);
const glm::fquat CCW_30deg_rotation_about_Y = glm::fquat(0.0f, -0.258819f, 0.0f, 0.9659258f);

const glm::fquat front_rotation = default_rotation;
const glm::fquat back_rotation = CCW_180_rotation_about_y;

const glm::fquat left_rotation = CCW_90_rotation_about_y;
const glm::fquat right_rotation = CW_90_rotation_about_y;

const glm::fquat floor_rotation = CW_90_rotation_about_x;
const glm::fquat ceiling_rotation = CCW_90_rotation_about_x;

const glm::fquat down_rotation = CW_90_rotation_about_x;
const glm::fquat up_rotation = CCW_90_rotation_about_x;

const glm::vec3 forward_direction(0.0f, 0.0f, -1.0f);
const glm::vec3 back_direction(0.0f, 0.0f, 1.0f);
const glm::vec3 left_direction(-1.0f, 0.0f, 0.0f);
const glm::vec3 right_direction(1.0f, 1.0f, 0.0f);
const glm::vec3 up_direction(0.0f, 1.0f, 0.0f);
const glm::vec3 down_direction(0.0f, -1.0f, 0.0f);

const glm::vec2 zero_vec2(0.0f, 0.0f);
const glm::vec2 one_vec2(1.0f, 1.0f);

const glm::vec3 zero_vec3(0.0f, 0.0f, 0.0f);
const glm::vec3 one_vec3(1.0f, 1.0f, 1.0f);

const glm::vec4 zero_vec4(0.0f, 0.0f, 0.0f, 1.0f);
const glm::vec4 one_vec4(1.0f, 1.0f, 1.0f, 1.0f);

struct GLMPose
{
    GLMPose()
    {
    }

    GLMPose(const glm::vec3& translation, const glm::fquat& rotation) :
    translation_(translation), rotation_(rotation)
    {
    }

    GLMPose(const glm::vec3& translation, const glm::vec3& euler_angle_deg) :
        translation_(translation), euler_angles_degrees_(euler_angle_deg)
    {
        update_rotation_from_euler();
    }

    glm::vec3 translation_ = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::fquat rotation_ = default_rotation;
    glm::vec3 scale_ = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 euler_angles_degrees_ = glm::vec3(0.0f, 0.0f, 0.0f);
    bool is_valid_ = true;

    uint64_t timestamp_ = 0;

    void clear();

    glm::mat4 to_matrix() const;

    void update_rotation_from_euler();

    void transform(const GLMPose &glm_pose);
    void project_to_horizontal_plane();
};

XrMatrix4x4f convert_to_xr(const glm::mat4& input);
glm::mat4 convert_to_glm(const XrMatrix4x4f& input);

glm::mat4 convert_to_rotation_matrix(const glm::fquat& rotation);

XrVector3f convert_to_xr(const glm::vec3 &input);

glm::vec3 convert_to_glm(const XrVector3f &input);

XrQuaternionf convert_to_xr(const glm::fquat &input);

glm::fquat convert_to_glm(const XrQuaternionf &input);

glm::mat4 convert_to_rotation_matrix(const glm::fquat &rotation);

GLMPose convert_to_glm_pose(const XrVector3f &position, const XrQuaternionf &rotation,
                       const XrVector3f &scale);

GLMPose convert_to_glm_pose(const XrPosef &xr_pose);

XrPosef convert_to_xr_pose(const GLMPose &glm_pose);

typedef uint32_t IndexType;

typedef glm::vec2 Vector2;
typedef glm::vec3 Vector3;
typedef glm::vec4 Vector4;

typedef Vector3 Position;
typedef Vector3 Normal;
typedef Vector4 Colour;
typedef Vector2 TexCoord;

const Colour white(1.0f, 1.0f, 1.0f, 1.0f);
const Colour black(0.0f, 0.0f, 0.0f, 1.0f);
const Colour transparent_white(1.0f, 1.0f, 1.0f, 0.0f);
const Colour transparent_black(0.0f, 0.0f, 0.0f, 0.0f);

const Colour red(1.0f, 0.0f, 0.0f, 1.0f);
const Colour green(0.0f, 1.0f, 0.0f, 1.0f);
const Colour blue(0.0f, 0.0f, 1.0f, 1.0f);

const Colour pluto_purple(0.224f, 0.196f, 0.502f, 1.0f);

static inline Colour apply_gamma_correction(const Colour& input, const float gamma)
{
	Colour output(powf(input.x, gamma), powf(input.y, gamma), powf(input.z, gamma), input.w);
	return output;
}

const Position default_position = zero_vec3;
const Normal default_normal = back_direction;
const Colour default_colour = white;
const TexCoord default_texcoord = zero_vec2;

typedef Position Vertex;

struct HitLocation
{
	float distance_ = FLT_MAX;
	glm::vec2 barycentric_;
	glm::vec2 uv_;
	glm::vec3 position_;
	glm::vec3 normal_;
};

struct IntersectionResult
{
	bool was_hit_ = false;
	HitLocation hit_;
};

struct Ray
{
	glm::vec3 origin_;
	glm::vec3 direction_;
};

struct ColouredVertex
{
	Position position_;
	Colour colour_;

	ColouredVertex()
	{
		position_ = default_position;
		colour_ = default_colour;
	};

	ColouredVertex(const Position& position)
	{
		position_ = position;
		colour_ = default_colour;
	};

	ColouredVertex(const Position& position, const Colour& colour)
	{
		position_ = position;
		colour_ = colour;
	};
};

struct TexturedVertex
{
	Position position_;
	TexCoord texcoord_;

	TexturedVertex()
	{
		position_ = default_position;
		texcoord_ = default_texcoord;
	};

	TexturedVertex(const Position& position, const TexCoord& texcoord)
	{
		position_ = position;
		texcoord_ = texcoord;
	};
};

struct ColouredTexturedVertex
{
	Position position_;
	Colour colour_;
	TexCoord texcoord_;

	ColouredTexturedVertex()
	{
		position_ = default_position;
		colour_ = default_colour;
		texcoord_ = default_texcoord;
	};

	ColouredTexturedVertex(const Position& position, const TexCoord& texcoord)
	{
		position_ = position;
		colour_ = default_colour;
		texcoord_ = texcoord;
	};

	ColouredTexturedVertex(const Position& position, const Colour& colour, const TexCoord& texcoord)
	{
		position_ = position;
		colour_ = colour;
		texcoord_ = texcoord;
	};
};

struct ColouredTexturedNormalVertex
{
	Position position_;
	Normal normal_;
	Colour colour_;
	TexCoord texcoord_;

	ColouredTexturedNormalVertex()
	{
		position_ = default_position;
		normal_ = default_normal;
		colour_ = default_colour;
		texcoord_ = default_texcoord;
	};

	ColouredTexturedNormalVertex(const Position& position, const Normal& normal, const Colour& colour,
		const TexCoord& texcoord)
	{
		position_ = position;
		normal_ = normal;
		colour_ = colour;
		texcoord_ = texcoord;
	};
};

const glm::vec2 UV00(0.0f, 0.0f);
const glm::vec2 UV01(0.0f, 1.0f);
const glm::vec2 UV10(1.0f, 0.0f);
const glm::vec2 UV11(1.0f, 1.0f);

struct TexturedTriangle
{
    TexturedVertex A_;
    TexturedVertex B_;
    TexturedVertex C_;

    TexturedTriangle()
            : A_()
            , B_()
            , C_(){};

    TexturedTriangle(const Position& A, const Position& B, const Position& C)
            : A_(A, UV00)
            , B_(B, UV01)
            , C_(C, UV11){};

    TexturedTriangle(const Position& A, const Position& B, const Position& C, const TexCoord& UV_A, const TexCoord& UV_B,
                     const TexCoord& UV_C)
            : A_(A, UV_A)
            , B_(B, UV_B)
            , C_(C, UV_C){};

    Normal compute_normal() const;
    TexturedTriangle transform(const glm::mat4& matrix) const;
    IntersectionResult intersect(const Ray& ray) const;
};

struct Quad
{
    TexturedTriangle A_;
    TexturedTriangle B_;

    Quad()
            : A_()
            , B_(){};

    Quad(const TexturedTriangle& A, const TexturedTriangle& B)
            : A_(A)
            , B_(B){};

    Quad(const Position& A, const Position& B, const Position& C, const Position& D)
            : A_(TexturedTriangle(A, B, C, UV01, UV11, UV10))
            , B_(TexturedTriangle(A, C, D, UV01, UV10, UV00)){};

    Quad(const Position& A, const TexCoord& UV_A, const Position& B, const TexCoord& UV_B, const Position& C,
         const TexCoord& UV_C, const Position& D, const TexCoord& UV_D)
            : A_(TexturedTriangle(A, B, C, UV_A, UV_B, UV_C))
            , B_(TexturedTriangle(A, C, D, UV_A, UV_C, UV_D)){};

    Normal compute_normal() const;
    Quad transform(const glm::mat4& matrix) const;
    IntersectionResult intersect(const Ray& ray) const;
};


    struct IntersectionQuery
    {
        Quad* quad_ = nullptr;
        BVR::GLMPose pose_;
        IntersectionResult* result_ = nullptr;
    };


} // BVR

#endif
