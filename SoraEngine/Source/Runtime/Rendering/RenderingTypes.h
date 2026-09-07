#pragma once

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Sora
{

	using Vec2 = glm::vec2;
	using Vec3 = glm::vec3;
	using Vec4 = glm::vec4;

	using IVec2 = glm::ivec2;
	using UVec2 = glm::uvec2;

	using Mat3 = glm::mat3;
	using Mat4 = glm::mat4;
	
	using Quaternion = glm::quat;

	struct alignas(16) UniformBufferObject {
		alignas(16) Mat4 ViewMatrix;
		alignas(16) Mat4 ProjectionMatrix;
		alignas(16) Mat4 ViewProjectionMatrix;
	};

	inline Mat4 Perspective(float InFovRadians, float InAspect, float InZNear, float InZFar) {
		Mat4 proj = glm::perspective(InFovRadians, InAspect, InZNear, InZFar);
        proj[1][1] *= -1.0f; // for Vulkan
        return proj;
	}

	inline Mat4 LookAt(const Vec3& InEye, const Vec3& InCenter, const Vec3& InUp) {
        return glm::lookAt(InEye, InCenter, InUp);
    }

} // namespace Sora
