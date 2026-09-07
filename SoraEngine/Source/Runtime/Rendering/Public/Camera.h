#pragma once

#include "Runtime/Rendering/RenderingTypes.h"
#include "Runtime/RHI/Public/RHIDevice.h"

namespace Sora::Rendering
{

	struct CameraDesc {
		float FOV = 60.0f;
		float AspectRatio = 1280.0f / 780.0f;
		float NearPlane = 0.1f;
		float FarPlane = 1000.0f;
	};

	class SORA_API Camera {
	public:
		explicit Camera(CameraDesc& InCameraDesc) : Desc(InCameraDesc) {}
		~Camera() = default;

		void SetPosition(const Vec3& InPosition);
		void Move(const Vec3& InTranslation);

		// Euler Angles
		void SetOrientation(float InPitchDegrees, float InYawDegrees, float InRollDegrees = 0.0f);
		void Rotate(float InPitchDegrees, float InYawDegrees, float InRollDegrees = 0.0f);

		// Quaternion Angles
		void SetOrientation(const Quaternion& InOrientation);
		void Rotate(const Quaternion& InDeltaRotation);

		void SetPerspective(float InFovDegrees, float InAspectRatio, float InNearPlane, float InFarPlane);

		void SetAspectRatio(float InAspectRatio);

		const Mat4& GetViewMatrix() const;

		const Mat4& GetProjectionMatrix() const;

		const Mat4& GetViewProjectionMatrix() const;

		const UniformBufferObject& GetUBO() const;

		inline Vec3 GetForwardVector() const { return glm::rotate(Orientation, Vec3(0.0f, 0.0f, -1.0f)); }
		inline Vec3 GetUpVector() const      { return glm::rotate(Orientation, Vec3(0.0f, 1.0f, 0.0f)); }
		inline Vec3 GetRightVector() const   { return glm::rotate(Orientation, Vec3(1.0f, 0.0f, 0.0f)); }
		inline Vec3 GetPosition() const      { return Position; }
		inline const CameraDesc& GetDesc() const	 { return Desc; }

		void UpdateUBO() const;

		void LookAt(const Vec3& InTarget, const Vec3& InUp = Vec3(0.0f, 1.0f, 0.0f));
		
	private:
		Vec3 Position{ 0.0f, 0.0f, 0.0f };
		Quaternion Orientation{ 1.0f, 0.0f, 0.0f, 0.0f };
		CameraDesc Desc{};

		mutable UniformBufferObject UBO{};
		mutable bool bViewDirty = true;
		mutable bool bProjectionDirty = true;
	};

} // namespace Sora
